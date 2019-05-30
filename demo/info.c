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


void com (int fd, uint16_t comid, void * data, size_t size8)
{
	uint16_t status = 0;
	int r;
	r = lep_i2c_com (fd, comid, data, size8, &status);
	printf ("%30s : id=%u, r=%s, status=%i, error=%i, busy=%i\n", "COMMAND", comid, lep_result_str (r), status, status >> 8, status & LEP_STATUS_BUSY);
}


void app_print_status (uint16_t status)
{
	//TODO: Should error be 8 bit?
	int error;
	//error = (status >> 8) | 0xFF00;
	error = status >> 8;
	char const * ci = TCOL (TCOL_BOLD, TCOL_YELLOW, TCOL_DEFAULT);
	char const * ce = NULL;
	if (error < 0) {ce = TCOL (TCOL_BOLD, TCOL_RED, TCOL_DEFAULT);}
	else {ce = TCOL (TCOL_BOLD, TCOL_GREEN, TCOL_DEFAULT);}
	printf ("%s%30s%s : %i\n", ci, "Status", TCOL_RESET, (int)status);
	printf ("%s%30s%s : %s%i\n", ci, "status >> 8 = Error", TCOL_RESET, ce, (int)error);
	printf ("%s%30s%s : %i\n", ci, "LEP_STATUS_BUSY", TCOL_RESET, (int)!!(status & LEP_STATUS_BUSY));
	printf ("%s%30s%s : %i\n", ci, "LEP_STATUS_BOOTMODE", TCOL_RESET, (int)!!(status & LEP_STATUS_BOOTMODE));
	printf ("%s%30s%s : %i\n", ci, "LEP_STATUS_BOOTSTATUS", TCOL_RESET, (int)!!(status & LEP_STATUS_BOOTSTATUS));
}


void app_print_temp (int fd)
{
	uint16_t tfpa = 0;
	uint16_t taux = 0;
	com (fd, LEP_COMID_FPATEMP, &tfpa, sizeof (uint16_t));
	com (fd, LEP_COMID_AUXTEMP, &taux, sizeof (uint16_t));
	printf ("%30s : %f\n", "LEP_COMID_FPATEMP", lep_to_celsius (tfpa));	
	printf ("%30s : %f\n", "LEP_COMID_AUXTEMP", lep_to_celsius (taux));
}


void app_print_uptime (int fd)
{
	uint32_t uptime = 0;
	com (fd, LEP_COMID_UPTIME, &uptime, sizeof (uint32_t));
	printf ("%30s : %u\n", "LEP_COMID_UPTIME", uptime);	
}


void app_print_shutter_ctrl (int fd)
{
	struct lep_shutter_ctrl ctrl = {0};
	com (fd, LEP_COMID_SHUTTER_CTRL, &ctrl, sizeof (struct lep_shutter_ctrl));
	printf ("%30s : %u\n", "mode", ctrl.mode);
	printf ("%30s : %u\n", "lockout", ctrl.lockout);
	printf ("%30s : %u\n", "fcc_freeze", ctrl.fcc_freeze);
	printf ("%30s : %u\n", "ffc_desired", ctrl.ffc_desired);
	printf ("%30s : %u\n", "fcc_time", ctrl.fcc_time);
	printf ("%30s : %u\n", "ffc_period", ctrl.ffc_period);
	printf ("%30s : %u\n", "open_explicit", ctrl.open_explicit);
	printf ("%30s : %u\n", "fcc_desired_deltatemp", ctrl.fcc_desired_deltatemp);
	printf ("%30s : %u\n", "imminent_delay", ctrl.imminent_delay);
}


void app_print_shutter_ctrl_mode (int fd)
{
	struct lep_shutter_ctrl ctrl = {0};
	com (fd, LEP_COMID_SHUTTER_CTRL, &ctrl, sizeof (struct lep_shutter_ctrl));
	printf ("%30s : %u\n", "mode", ctrl.mode);
}


void app_set_shutter_ctrl_mode (int fd, enum lep_shutter_mode mode)
{
	struct lep_shutter_ctrl ctrl = {0};
	com (fd, LEP_COMID_SHUTTER_CTRL | LEP_COMTYPE_GET, &ctrl, sizeof (struct lep_shutter_ctrl));
	ctrl.mode = mode;
	com (fd, LEP_COMID_SHUTTER_CTRL | LEP_COMTYPE_SET, &ctrl, sizeof (struct lep_shutter_ctrl));
}


void app_set_shutter_pos (int fd, enum lep_shutter_pos pos)
{
	com (fd, LEP_COMID_SHUTTER_CTRL | LEP_COMTYPE_SET, &pos, sizeof (uint32_t));
}


void app_print_shutter_pos (int fd)
{
	uint32_t pos;
	com (fd, LEP_COMID_SHUTTER_CTRL, &pos, sizeof (pos));
	printf ("%30s : %u\n", "pos", pos);
}


void app_set_gpio (int fd, uint16_t mode)
{
	com (fd, LEP_COMID_GPIO | LEP_COMTYPE_SET, &mode, sizeof (uint16_t));
}


void app_set_vsync_delay (int fd, int32_t d)
{
	com (fd, LEP_COMID_VSYNC_DELAY | LEP_COMTYPE_SET, &d, sizeof (d));
}


void app_set_vsync_delaystr (int fd, char const * d)
{
	intmax_t j;
	char * endptr;
	j = strtoimax (d, &endptr, 10);
	app_set_vsync_delay (fd, (int32_t)j);
}


void app_print_gpio (int fd)
{
	uint16_t mode = 0;
	com (fd, LEP_COMID_GPIO | LEP_COMTYPE_GET, &mode, sizeof (mode));
	printf ("%30s : %i\n", "LEP_COMID_GPIO", (int) mode);
}


void app_print_vsync_delay (int fd)
{
	int32_t d = 0;
	com (fd, LEP_COMID_VSYNC_DELAY | LEP_COMTYPE_GET, &d, sizeof (d));
	printf ("%30s : %i\n", "LEP_COMID_VSYNC_DELAY", (int) d);
}





int main (int argc, char * argv [])
{ 
	int fd = lep_i2c_open (LEP_I2C_DEV_RPI3);
	
	while (1)
	{
		int C = getopt (argc, argv, "hsturDd:v:cm:mp:");
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
			printf ("-u      : Print uptime\n");
			printf ("-r      : Reboot\n");
			printf ("-D      : Sleep one second\n");
			printf ("-c      : Print shutter ctrl\n");
			printf ("-mq     : Print mode\n");
			printf ("-m0     : Shutter mode manual\n");
			printf ("-m1     : Shutter mode auto\n");
			printf ("-m2     : Shutter mode external\n");
			printf ("-pq     : Print shutter position\n");
			printf ("-p0     : Shutter position idle\n");
			printf ("-p1     : Shutter position open\n");
			printf ("-p2     : Shutter position closed\n");
			printf ("-p2     : Shutter position breakon\n");
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
			com (fd, LEP_COMID_REBOOT | LEP_COMTYPE_RUN, NULL, 0);
			break;
			
			case 'u':
			app_print_uptime (fd);
			break;
			
			case 'D':
			sleep (1);
			break;
			
			case 'c':
			app_print_shutter_ctrl (fd);
			break;
			
			case 'm':
			if (optarg == NULL) {break;}
			if (optarg [0] == 'q')
			{
				app_print_shutter_ctrl_mode (fd);
			}
			else if (optarg [0] == '0')
			{
				app_set_shutter_ctrl_mode (fd, LEP_SHUTTER_MANUAL);
			}
			else if (optarg [0] == '1')
			{
				app_set_shutter_ctrl_mode (fd, LEP_SHUTTER_AUTO);
			}
			else if (optarg [0] == '2')
			{
				app_set_shutter_ctrl_mode (fd, LEP_SHUTTER_EXTERNAL);
			}
			break;
			
			case 'p':
			if (optarg == NULL) {break;}
			if (optarg [0] == 'q')
			{
				app_print_shutter_pos (fd);
			}
			else if (optarg [0] == '0')
			{
				app_set_shutter_pos (fd, LEP_SHUTTER_IDLE);
			}
			else if (optarg [0] == '1')
			{
				app_set_shutter_pos (fd, LEP_SHUTTER_OPEN);
			}
			else if (optarg [0] == '2')
			{
				app_set_shutter_pos (fd, LEP_SHUTTER_CLOSED);
			}
			else if (optarg [0] == '3')
			{
				app_set_shutter_pos (fd, LEP_SHUTTER_BRAKEON);
			}
			break;
			
			default:
			break;
		}
	}
	


	return 0;
	
main_error:
	
	return 1;
}
