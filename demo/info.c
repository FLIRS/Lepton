#define LEP_ASSERT ASSERT
#define LEP_ASSERT_C ASSERT_C
#define LEP_ASSERT_F ASSERT_F
#define LEP_ASSERT_CF ASSERT_CF

//#define LEP_TRACE TRACE
//#define LEP_TRACE_F TRACE_F
//#define LEP_TRACE_CF TRACE_CF


#include "debug.h"
#include "lep.h"
#include "tcol.h"
#include "common.h"


//printf
#include <stdio.h>

//timerfd_create
#include <sys/timerfd.h>

//read
#include <unistd.h>

//strerror
#include <string.h>

//errrno
#include <errno.h>

//intmax_t strtoimax(const char *nptr, char **endptr, int base);
#include <inttypes.h>


int app_print_temp (int dev)
{
	uint16_t status = 0;
	uint16_t fpa_temp = 0;
	uint16_t aux_temp = 0;
	int R;
	R = lep_i2c_com (dev, LEP_COMID_FPATEMP, &fpa_temp, sizeof (uint16_t), &status);
	//R = lep_i2c_com (dev, LEP_COMID_AUXTEMP | LEP_COMTYPE_RUN, &aux_temp, sizeof (uint16_t), &status);
	R = lep_i2c_com (dev, LEP_COMID_AUXTEMP, &aux_temp, sizeof (uint16_t), &status);
	printf ("%30s : %f\n", "LEP_COMID_FPATEMP", (fpa_temp / 100.0f) - 273.15f);	
	printf ("%30s : %f\n", "LEP_COMID_AUXTEMP", (aux_temp / 100.0f) - 273.15f);
	return R;
}






int app_set_gpio (int dev, uint16_t mode)
{
	uint16_t status = 0;
	int res;
	res = lep_i2c_com (dev, LEP_COMID_GPIO | LEP_COMTYPE_SET, &mode, sizeof (uint16_t), &status);
	app_print_status (status);
	return res;
}


int app_set_vsync_delay (int dev, int32_t d)
{
	uint16_t status = 0;
	int res;
	res = lep_i2c_com (dev, LEP_COMID_VSYNC_DELAY | LEP_COMTYPE_SET, &d, sizeof (d), &status);
	app_print_status (status);
	return res;
}


int app_set_vsync_delay_str (int dev, char const * d)
{
	intmax_t j;
	int res = 0;
	char * endptr;
	j = strtoimax (d, &endptr, 10);
	//if (-3 <= j && j <= 3)
	{
		res = app_set_vsync_delay (dev, (int32_t)j);
	}
	return res;
}


int app_print_gpio (int dev)
{
	uint16_t status = 0;
	uint16_t mode = 0;
	int res;
	res = lep_i2c_com (dev, LEP_COMID_GPIO | LEP_COMTYPE_GET, &mode, sizeof (mode), &status);
	printf ("%30s : %i\n", "LEP_COMID_GPIO", (int) mode);
	return res;
}


int app_print_vsync_delay (int dev)
{
	uint16_t status = 0;
	int32_t d = 0;
	int res;
	res = lep_i2c_com (dev, LEP_COMID_VSYNC_DELAY | LEP_COMTYPE_GET, &d, sizeof (d), &status);
	printf ("%30s : %i\n", "LEP_COMID_VSYNC_DELAY", (int) d);
	return res;
}





int main (int argc, char * argv [])
{ 
	int dev = lep_i2c_open (LEP_I2C_DEV_RPI3);
	
	while (1)
	{
		int C = getopt (argc, argv, "hstrd:v:");
		printf ("%s", "----------------------------------------------------\n");
		if (C == -1) {break;}
		switch (C)
		{
			case '?':
			goto main_error;
			break;
			
			case 'h':
			printf ("-h      : Help\n");
			printf ("-s      : Print status\n");
			printf ("-v<0,1> : Enable/disable vsync\n");
			printf ("-d      : Set vsync delay\n");
			printf ("-t      : Print temperature\n");
			printf ("-r      : Reboot\n");
			break;
			
			case 's':
			app_print_status (app_status (dev));
			break;
			
			case 'v':
			if (optarg == NULL) {break;}
			if (optarg [0] == '1')
			{
				app_print_gpio (dev);
				app_set_gpio (dev, LEP_GPIO_VSYNC);
				app_print_gpio (dev);
			}
			if (optarg [0] == '0')
			{
				app_print_gpio (dev);
				app_set_gpio (dev, LEP_GPIO_MODE);
				app_print_gpio (dev);
			}
			break;
			
			case 'd':
			if (optarg == NULL) {break;}
			app_print_vsync_delay (dev);
			app_set_vsync_delay_str (dev, optarg);
			app_print_vsync_delay (dev);
			break;
			
			case 't':
			app_print_temp (dev);
			sleep (1);
			break;
			
			case 'r':
			app_reboot (dev);
			break;
			
			default:
			break;
		}
	}
	


	return 0;
	
main_error:
	
	return 1;
}
