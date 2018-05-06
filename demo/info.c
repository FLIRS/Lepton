#define LEP_ASSERT_ACF(A,C,F,...) ASSERT_ACF(A,C,F,__VA_ARGS__)


#include "assert_extended.h"
#include "lep.h"
#include "crc.h"


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


int app_print_status (int dev)
{
	uint16_t status = 0;
	int16_t error;
	int R;
	R = lep_i2c_read1 (dev, LEP_REG_STATUS, &status);
	error = (status >> 8) | 0xFF00; //No error = -256????
	printf ("%30s : %i\n", "Status", (int)status);
	printf ("%30s : %i\n", "Error", (int)error);
	printf ("%30s : %i\n", "LEP_STATUS_BUSY", (int)!!(status & LEP_STATUS_BUSY));
	printf ("%30s : %i\n", "LEP_STATUS_BOOTMODE", (int)!!(status & LEP_STATUS_BOOTMODE));
	printf ("%30s : %i\n", "LEP_STATUS_BOOTSTATUS", (int)!!(status & LEP_STATUS_BOOTSTATUS));
	return R;
}


int app_set_gpio (int dev, uint16_t mode)
{
	uint16_t status = 0;
	int res;
	res = lep_i2c_com (dev, LEP_COMID_GPIO | LEP_COMTYPE_SET, &mode, sizeof (uint16_t), &status);
	return res;
}

int app_print_gpio (int dev)
{
	uint16_t status = 0;
	uint16_t mode = 0;
	int res;
	res = lep_i2c_com (dev, LEP_COMID_GPIO | LEP_COMTYPE_GET, &mode, sizeof (uint16_t), &status);
	printf ("%30s : %i\n", "LEP_COMID_GPIO", (int) mode);
	return res;
}



int main (int argc, char * argv [])
{ 

	int dev = lep_i2c_open (LEP_I2C_DEV_RPI3);
	printf ("dev: %i\n", dev);

	int C = 1;
	while (C != -1)
	{
		C = getopt (argc, argv, "stv:");
		switch (C)
		{
			case '?':
			goto main_error;
			break;
			
			case 's':
			app_print_status (dev);
			break;
			
			case 'v':
			if (optarg && optarg [0] == '1')
			{
				app_print_gpio (dev);
				app_set_gpio (dev, LEP_GPIO_VSYNC);
				app_print_gpio (dev);
			}
			if (optarg && optarg [0] == '0')
			{
				app_print_gpio (dev);
				app_set_gpio (dev, LEP_GPIO_MODE);
				app_print_gpio (dev);
			}
			break;
			
			case 't':
			app_print_temp (dev);
			break;
			
			default:
			break;
		}
	}
	


	return 0;
	
main_error:
	
	return 1;
}
