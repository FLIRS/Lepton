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


int app_print_temp (int fd)
{
	uint16_t status = 0;
	uint16_t tfpa = 0;
	uint16_t taux = 0;
	int r;
	r = lep_i2c_com (fd, LEP_COMID_FPATEMP, &tfpa, sizeof (uint16_t), &status);
	r = lep_i2c_com (fd, LEP_COMID_AUXTEMP, &taux, sizeof (uint16_t), &status);
	printf ("%30s : %f\n", "LEP_COMID_FPATEMP", lep_to_celsius (tfpa));	
	printf ("%30s : %f\n", "LEP_COMID_AUXTEMP", lep_to_celsius (taux));
	return r;
}


int app_set_gpio (int fd, uint16_t mode)
{
	uint16_t status = 0;
	int r;
	r = lep_i2c_com (fd, LEP_COMID_GPIO | LEP_COMTYPE_SET, &mode, sizeof (uint16_t), &status);
	return r;
}


int app_set_vsync_delay (int fd, int32_t d)
{
	uint16_t status = 0;
	int r;
	r = lep_i2c_com (fd, LEP_COMID_VSYNC_DELAY | LEP_COMTYPE_SET, &d, sizeof (d), &status);
	return r;
}


int app_set_vsync_delaystr (int fd, char const * d)
{
	intmax_t j;
	int r = 0;
	char * endptr;
	j = strtoimax (d, &endptr, 10);
	//if (-3 <= j && j <= 3)
	{
		r = app_set_vsync_delay (fd, (int32_t)j);
	}
	return r;
}


int app_print_gpio (int fd)
{
	uint16_t status = 0;
	uint16_t mode = 0;
	int r;
	r = lep_i2c_com (fd, LEP_COMID_GPIO | LEP_COMTYPE_GET, &mode, sizeof (mode), &status);
	printf ("%30s : %i\n", "LEP_COMID_GPIO", (int) mode);
	return r;
}


int app_print_vsync_delay (int fd)
{
	uint16_t status = 0;
	int32_t d = 0;
	int r;
	r = lep_i2c_com (fd, LEP_COMID_VSYNC_DELAY | LEP_COMTYPE_GET, &d, sizeof (d), &status);
	printf ("%30s : %i\n", "LEP_COMID_VSYNC_DELAY", (int) d);
	return r;
}





int main (int argc, char * argv [])
{ 
	int fd = lep_i2c_open (LEP_I2C_DEV_RPI3);
	
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
			printf ("-v0     : Disable vsync\n");
			printf ("-v1     : Enable vsync\n");
			printf ("-vq     : Print vsync\n");
			printf ("-d<n>   : Set vsync delay\n");
			printf ("-dq     : Print vsync delay\n");
			printf ("-t      : Print temperature\n");
			printf ("-r      : Reboot\n");
			break;
			
			case 's':
			app_print_status (lep_i2c_status (fd));
			break;
			
			case 'v':
			if (optarg == NULL) {break;}
			if (optarg [0] == 'q')
			{
				app_print_gpio (fd);
			}
			if (optarg [0] == '1')
			{
				app_set_gpio (fd, LEP_GPIO_VSYNC);
			}
			if (optarg [0] == '0')
			{
				app_set_gpio (fd, LEP_GPIO_MODE);
			}
			break;
			
			case 'd':
			if (optarg == NULL) {break;}
			if (optarg [0] == 'q')
			{
				app_print_vsync_delay (fd);
			}
			else
			{
				app_set_vsync_delaystr (fd, optarg);
			}
			break;
			
			case 't':
			app_print_temp (fd);
			sleep (1);
			break;
			
			case 'r':
			app_reboot (fd);
			break;
			
			default:
			break;
		}
	}
	


	return 0;
	
main_error:
	
	return 1;
}
