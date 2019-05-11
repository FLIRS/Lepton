/**
 * @file   lepmod.c
 * @author Johan Söderlind Åström
 * @date   2019-05-10
 * @brief  A kernel module for FLIR Lepton a thermal camera that is connected to
 * a GPIO, SPI, I2C. It has full support for interrupts and for sysfs entries so that an interface
 * can be created to the camera or the camera can be configured from Linux userspace.
 * The sysfs entry appears at /sys/lepmod/gpio17
 * @see https://github.com/FLIRS/Lepton
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/kobject.h>
#include <linux/time.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johan Söderlind Åström");
MODULE_DESCRIPTION("A FLIR Lepton driver for Raspberry PI 3");
MODULE_VERSION("0.1");

static unsigned int vsync_pin = 17;
// S_IRUGO can be read/not changed
module_param(vsync_pin, uint, S_IRUGO);
MODULE_PARM_DESC(vsync_pin, "vsync pin (default=17)");

static char   vsync_pinname[8] = "gpioXXX";
static int    vsync_irqnr;
static int    vsync_n = 0;
static struct timespec ts_last, ts_current, ts_diff;

// Function prototype for the custom IRQ handler function -- see below for the implementation
static irq_handler_t  lepmod_irq_handler (unsigned int irq, void *dev_id, struct pt_regs *regs);

/** @brief A callback function to output the vsync_n variable
 *  @param kobj represents a kernel object device that appears in the sysfs filesystem
 *  @param attr the pointer to the kobj_attribute struct
 *  @param buf the buffer to which to write the number of presses
 *  @return return the total number of characters written to the buffer (excluding null)
 */
static ssize_t vsync_n_show (struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", vsync_n);
}

/** @brief A callback function to read in the vsync_n variable
 *  @param kobj represents a kernel object device that appears in the sysfs filesystem
 *  @param attr the pointer to the kobj_attribute struct
 *  @param buf the buffer from which to read the number of presses (e.g., reset to 0).
 *  @param count the number characters in the buffer
 *  @return return should return the total number of characters used from the buffer
 */
static ssize_t vsync_n_store
(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	sscanf (buf, "%du", &vsync_n);
	return count;
}

/** @brief Displays the last time the button was pressed -- manually output the date (no localization) */
static ssize_t lastTime_show 
(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	char const format [] = "%.2lu:%.2lu:%.2lu:%.9lu \n";
	long unsigned h = (ts_last.tv_sec/3600)%24;
	long unsigned m = (ts_last.tv_sec/60) % 60;
	long unsigned s = ts_last.tv_sec % 60;
	return sprintf (buf, format, h, m, s, ts_last.tv_nsec);
}

/** @brief Display the time difference in the form secs.nanosecs to 9 places */
static ssize_t diffTime_show
(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	char const format [] = "%lu.%.9lu\n";
	return sprintf (buf, format, ts_diff.tv_sec, ts_diff.tv_nsec);
}



/**  Use these helper macros to define the name and access levels of the kobj_attributes
 *  The kobj_attribute has an attribute attr (name and mode), show and store function pointers
 *  The count variable is associated with the vsync_n variable and it is to be exposed
 *  with mode 0666 using the vsync_n_show and vsync_n_store functions above
 */
static struct kobj_attribute count_attr = __ATTR(vsync_n, 0444, vsync_n_show, vsync_n_store);

/**  The __ATTR_RO macro defines a read-only attribute. There is no need to identify that the
 *  function is called _show, but it must be present. __ATTR_WO can be  used for a write-only
 *  attribute but only in Linux 3.11.x on.
 */
// the last time pressed kobject attr
static struct kobj_attribute time_attr  = __ATTR_RO(lastTime);
// the difference in time attr
static struct kobj_attribute diff_attr  = __ATTR_RO(diffTime);

/**  The lepmod_attrs[] is an array of attributes that is used to create the attribute group below.
 *  The attr property of the kobj_attribute is used to extract the attribute struct
 */
static struct attribute *lepmod_attrs [] = 
{
	// The number of button presses
	&count_attr.attr,
	// Time of the last button press in HH:MM:SS:NNNNNNNNN
	&time_attr.attr,
	// The difference in time between the last two presses
	&diff_attr.attr,
	NULL,
};

/**  The attribute group uses the attribute array and a name, which is exposed on sysfs -- in this
 *  case it is gpio115, which is automatically defined in the lepmod_init() function below
 *  using the custom kernel parameter that can be passed when the module is loaded.
 */
static struct attribute_group attr_group = 
{
	// The name is generated in lepmod_init()
	.name  = vsync_pinname,
	// The attributes array defined just above
	.attrs = lepmod_attrs,
};

static struct kobject *ebb_kobj;

/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point. In this example this
 *  function sets up the GPIOs and the IRQ
 *  @return returns 0 if successful
 */
static int __init lepmod_init (void)
{
	int result = 0;

	printk (KERN_INFO "lepmod: Initializing the lepmod LKM\n");
	
	// Create the gpio17 name for /sys/lepmod/gpio17
	// create the kobject sysfs entry at /sys/ebb -- probably not an ideal location!
	// kernel_kobj points to /sys/kernel
	sprintf (vsync_pinname, "gpio%d", vsync_pin);
	ebb_kobj = kobject_create_and_add ("lepmod", kernel_kobj->parent);
	if(!ebb_kobj)
	{
		printk (KERN_ALERT "lepmod: failed to create kobject mapping\n");
		return -ENOMEM;
	}
	
	// add the attributes to /sys/lepmod/ -- for example, /sys/lepmod/gpio17/vsync_n
	result = sysfs_create_group (ebb_kobj, &attr_group);
	if(result)
	{
		// clean up -- remove the kobject sysfs entry
		printk (KERN_ALERT "lepmod: failed to create sysfs group\n");
		kobject_put(ebb_kobj);
		return result;
	}
	
	// set the last time to be the current time
	// set the initial time difference to be 0
	getnstimeofday (&ts_last);
	ts_diff = timespec_sub (ts_last, ts_last);

	// Set up the vsync_pin
	// Set the button GPIO to be an input
	// Causes gpio17 to appear in /sys/class/gpio
	// the bool argument prevents the direction from being changed
	gpio_request (vsync_pin, "sysfs");
	gpio_direction_input (vsync_pin);
	gpio_export (vsync_pin, false);
	
	// Perform a quick test to see that the gpiopin is working as expected on LKM load
	printk (KERN_INFO "lepmod: The gpio%d value is %d\n", vsync_pin, gpio_get_value (vsync_pin));
	
	// GPIO numbers and IRQ numbers are not the same! This function performs the mapping for us
	vsync_irqnr = gpio_to_irq (vsync_pin);
	printk (KERN_INFO "lepmod: The gpio%d is mapped to IRQ: %d\n", vsync_pin, vsync_irqnr);
	
	// This next call requests an interrupt line
	// The interrupt number requested
	// The pointer to the handler function below
	// Use the custom kernel param to set interrupt type
	// Used in /proc/interrupts to identify the owner
	// The *dev_id for shared interrupt lines, NULL is okay
	result = request_irq (vsync_irqnr,(irq_handler_t) lepmod_irq_handler, IRQF_TRIGGER_RISING, "lepmod_irq_handler",  NULL);
	return result;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit lepmod_exit(void)
{
	// clean up -- remove the kobject sysfs entry
	// Free the IRQ number, no *dev_id required in this case
	// Unexport the GPIO
	// Free the GPIO
	printk (KERN_INFO "lepmod: The number of vsync was %d times\n", vsync_n);
	kobject_put (ebb_kobj);
	free_irq (vsync_irqnr, NULL); 
	gpio_unexport (vsync_pin);
	gpio_free (vsync_pin);
	printk (KERN_INFO "lepmod: Goodbye from the lepmod LKM!\n");
}

/** @brief The GPIO IRQ Handler function
 *  This function is a custom interrupt handler that is attached to the GPIO above. The same interrupt
 *  handler cannot be invoked concurrently as the interrupt line is masked out until the function is complete.
 *  This function is static as it should not be invoked directly from outside of this file.
 *  @param irq    the IRQ number that is associated with the GPIO -- useful for logging.
 *  @param dev_id the *dev_id that is provided -- can be used to identify which device caused the interrupt
 *  Not used in this example as NULL is passed.
 *  @param regs   h/w specific register values -- only really ever used for debugging.
 *  return returns IRQ_HANDLED if successful -- should return IRQ_NONE otherwise.
 */
static irq_handler_t lepmod_irq_handler (unsigned int irq, void *dev_id, struct pt_regs *regs)
{
	// Get the current time as ts_current
	// Determine the time difference between last 2 events
	// Store the current time as the last time ts_last
	getnstimeofday (&ts_current);
	ts_diff = timespec_sub (ts_current, ts_last);
	ts_last = ts_current;                
	//printk (KERN_INFO "lepmod: The vsync_pin %d state is currently: %d\n", pin, gpio_get_value(vsync_pin));
	vsync_n ++;
	return (irq_handler_t) IRQ_HANDLED;
}

// This next calls are  mandatory -- they identify the initialization function
// and the cleanup function (as above).
module_init(lepmod_init);
module_exit(lepmod_exit);