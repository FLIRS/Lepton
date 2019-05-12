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
#include <linux/fs.h>
#include <linux/pci.h>
#include <linux/spi/spi.h>
#include <linux/dma-mapping.h>
#include <linux/pci_ids.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include "lep.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johan Söderlind Åström");
MODULE_DESCRIPTION("A FLIR Lepton driver for Raspberry PI 3");
MODULE_VERSION("0.1");

#define BUF_SIZE 164
#define BITS_PER_WORD 8
#define LEPTON_SPEED_HZ 12500000
#define DEVICE_NAME "lepton"
#define CLASS_NAME "lepton"

static unsigned int vsync_pin = 17;
// S_IRUGO can be read/not changed
module_param(vsync_pin, uint, S_IRUGO);
MODULE_PARM_DESC(vsync_pin, "vsync pin (default=17)");

static char   vsync_pinname[8] = "gpioXXX";
static int    vsync_irqnr;
static int    vsync_n = 0;
static struct timespec ts_last, ts_current, ts_diff;
static struct spi_device* spi_dev = NULL;
static int    major_number;
static struct class *leptonClass = NULL;
static struct device *leptonDevice = NULL;
static struct lep_packet packet [LEP2_HEIGHT] = {0};
static struct lep_packet packet1 = {0};
struct spi_transfer xfers [1];
struct spi_message message;

static ssize_t vsync_n_show (struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", vsync_n);
}

static ssize_t vsync_n_store
(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	sscanf (buf, "%du", &vsync_n);
	return count;
}


static ssize_t lastTime_show 
(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	char const format [] = "%.2lu:%.2lu:%.2lu:%.9lu \n";
	long unsigned h = (ts_last.tv_sec/3600)%24;
	long unsigned m = (ts_last.tv_sec/60) % 60;
	long unsigned s = ts_last.tv_sec % 60;
	return sprintf (buf, format, h, m, s, ts_last.tv_nsec);
}

static ssize_t diffTime_show
(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	char const format [] = "%lu.%.9lu\n";
	return sprintf (buf, format, ts_diff.tv_sec, ts_diff.tv_nsec);
}

static ssize_t packet1_show
(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	char const format [] = "%02x %02x %04x\n";
	return sprintf (buf, format, packet1.reserved, packet1.number, packet1.checksum);
}



static struct kobj_attribute count_attr = __ATTR(vsync_n, 0444, vsync_n_show, vsync_n_store);
static struct kobj_attribute time_attr  = __ATTR_RO(lastTime);
static struct kobj_attribute diff_attr  = __ATTR_RO(diffTime);
static struct kobj_attribute packet1_attr  = __ATTR_RO(packet1);

static struct attribute *lepmod_attrs [] = 
{
	&count_attr.attr,
	&time_attr.attr,
	&diff_attr.attr,
	&packet1_attr.attr,
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

// Function prototype for the custom IRQ handler function -- see below for the implementation
static irqreturn_t lepmod_irq_handler (int irq, void * dev_id);





static int dev_open (struct inode *ignore1, struct file *ignore2)
{
	return 0;
}

static int dev_release (struct inode *ignore1, struct file *ignore2)
{
	return 0;
}

static ssize_t dev_read (struct file *f, char *out, size_t len, loff_t *off)
{
	int result = 0;
	return result;
}



static struct file_operations fops =
{
	.open = dev_open,
	.read = dev_read,
	.release = dev_release,
};


static int lepton_driver_list (struct device *dev, void *n)
{
	const struct spi_device *spi = to_spi_device(dev);
	printk(KERN_INFO "Found spi device [%s]\n", spi->modalias);
	return !strcmp(spi->modalias, "spidev");
}


static int __init lepmod_init (void)
{
	int result = 0;
	struct spi_master *master = NULL;
	u16 i;
	printk (KERN_INFO "lepmod: Initializing the lepmod LKM\n");
	
	memset (&xfers [0], 0, sizeof(struct spi_transfer));
	memset (&message, 0, sizeof (struct spi_message));
	xfers [0].rx_buf = kmalloc (LEP_PACKET_SIZE, GFP_DMA | GFP_KERNEL);
	if(!xfers [0].rx_buf)
	{
		printk(KERN_ALERT "Failed to allocate %d bytes for lepton\n", (int)LEP_PACKET_SIZE);
		return -EBUSY;
	}
	xfers [0].len = LEP_PACKET_SIZE;
	memset (xfers [0].rx_buf, 0, LEP_PACKET_SIZE);
	xfers [0].speed_hz = LEPTON_SPEED_HZ;
	xfers [0].bits_per_word = BITS_PER_WORD;
	xfers [0].cs_change = 0;
	xfers [0].delay_usecs = 10;

	for (i = 0; i < 10 && !spi_dev; ++i)
	{
		master = spi_busnum_to_master (i);
		if(master == NULL) {continue;}
		printk(KERN_INFO "Found master on bus %d\n", (int)i);
		spi_dev = (struct spi_device*) device_find_child (&master->dev, NULL, lepton_driver_list);
	}
	if(!spi_dev)
	{
		printk(KERN_ALERT "No spidev found for lepton module\n");
		return -ENODEV;
	}
	major_number = register_chrdev (0, DEVICE_NAME, &fops);
	if(major_number < 0)
	{
		printk(KERN_ALERT "Failed to register lepton major number!\n");
		return major_number;
	}
	leptonClass = class_create (THIS_MODULE, CLASS_NAME);
	if (IS_ERR(leptonClass))
	{
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register lepton device class!\n");
		return PTR_ERR(leptonClass);
	}
	leptonDevice = device_create (leptonClass, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
	if (IS_ERR(leptonDevice))
	{
		class_unregister (leptonClass);
		class_destroy (leptonClass);
		unregister_chrdev (major_number, DEVICE_NAME);
		printk (KERN_ALERT "Failed to create lepton device!\n");
		return PTR_ERR(leptonDevice);
	}
	printk (KERN_INFO "Lepton intialized\n");
	sprintf (vsync_pinname, "gpio%d", vsync_pin);
	ebb_kobj = kobject_create_and_add ("lepmod", kernel_kobj->parent);
	if(!ebb_kobj)
	{
		printk (KERN_ALERT "lepmod: failed to create kobject mapping\n");
		return -ENOMEM;
	}
	result = sysfs_create_group (ebb_kobj, &attr_group);
	if(result)
	{
		printk (KERN_ALERT "lepmod: failed to create sysfs group\n");
		kobject_put(ebb_kobj);
		return result;
	}
	getnstimeofday (&ts_last);
	ts_diff = timespec_sub (ts_last, ts_last);
	gpio_request (vsync_pin, "sysfs");
	gpio_direction_input (vsync_pin);
	gpio_export (vsync_pin, false);
	printk (KERN_INFO "lepmod: The gpio%d value is %d\n", vsync_pin, gpio_get_value (vsync_pin));
	vsync_irqnr = gpio_to_irq (vsync_pin);
	printk (KERN_INFO "lepmod: The gpio%d is mapped to IRQ: %d\n", vsync_pin, vsync_irqnr);
	result = request_irq (vsync_irqnr, lepmod_irq_handler, IRQF_TRIGGER_RISING, "lepmod_irq_handler",  NULL);
	if (result)
	{
		printk (KERN_ERR "lepmod: cannot register IRQ %d\n", vsync_irqnr);
	}
	return result;
}


static void __exit lepmod_exit(void)
{
	printk (KERN_INFO "lepmod: The number of vsync was %d times\n", vsync_n);
	kobject_put (ebb_kobj);
	free_irq (vsync_irqnr, NULL); 
	gpio_unexport (vsync_pin);
	gpio_free (vsync_pin);
    printk (KERN_INFO "Cleaning up lepton module.\n");
    device_destroy (leptonClass, MKDEV(major_number, 0));
    class_unregister (leptonClass);
    class_destroy (leptonClass);
    unregister_chrdev (major_number, DEVICE_NAME);
	if (xfers[0].rx_buf)
	{
		kfree (xfers[0].rx_buf);
	}
	printk (KERN_INFO "lepmod: Goodbye from the lepmod LKM!\n");
}


static void read_complete (void *arg)
{
	complete (arg);
}

static int read_test (void)
{
	int status;
	DECLARE_COMPLETION_ONSTACK (context);
	
	spi_message_init (&message);
	spi_message_add_tail (&xfers [0], &message);
	message.complete = read_complete;
	message.context = &context;
	status = spi_async (spi_dev, &message);
	if (status < 0)
	{
		printk (KERN_ALERT "lepmod: spi_async not ok\n");
		disable_irq_nosync (vsync_irqnr); 
	}
	return status;
}


static irqreturn_t lepmod_irq_handler (int irq, void * dev_id)
{
	disable_irq_nosync (irq); 
	getnstimeofday (&ts_current);
	ts_diff = timespec_sub (ts_current, ts_last);
	ts_last = ts_current;                
	//vsync_n ++;
	read_test ();
	return IRQ_HANDLED;
}

module_init(lepmod_init);
module_exit(lepmod_exit);