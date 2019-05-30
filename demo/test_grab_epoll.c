#define LEP_ASSERT ASSERT
#define LEP_ASSERT_C ASSERT_C
#define LEP_ASSERT_F ASSERT_F
#define LEP_ASSERT_CF ASSERT_CF

//#define LEP_TRACE TRACE
//#define LEP_TRACE_F TRACE_F
//#define LEP_TRACE_CF TRACE_CF

#include "debug.h"
#include "lep.h"
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


#include <sys/epoll.h>


void com (int fd, uint16_t comid, void * data, size_t size8)
{
	uint16_t status = 0;
	int r;
	r = lep_i2c_com (fd, comid, data, size8, &status);
	printf ("%30s : id=%u, r=%s, status=%i, error=%i, busy=%i\n", "COMMAND", comid, lep_result_str (r), status, status >> 8, status & LEP_STATUS_BUSY);
}

void reboot (int fd)
{
	com (fd, LEP_COMID_REBOOT | LEP_COMTYPE_RUN, NULL, 0);
	uint16_t mode = LEP_GPIO_VSYNC;
	com (fd, LEP_COMID_GPIO | LEP_COMTYPE_SET, &mode, sizeof (mode));
	struct lep_shutter_ctrl ctrl = {0};
	ctrl.mode = LEP_SHUTTER_MANUAL;
	com (fd, LEP_COMID_SHUTTER_CTRL | LEP_COMTYPE_SET, &ctrl, sizeof (ctrl));
}


int main (int argc, char * argv [])
{
	while (1)
	{
		int c = getopt (argc, argv, "");
		if (c < 0) {break;}
		switch (c)
		{	
			default:
			break;
		}
	}
	
	int fde = epoll_create1 (0);
	int fdt = timerfd_create (CLOCK_MONOTONIC, 0);
	int fdg = lep_create_gpiofd (17);
	
	int fdspi = lep_spi_open (LEP_SPI_DEV_RPI3);
	int fdi2c = lep_i2c_open (LEP_I2C_DEV_RPI3);
	
	ASSERT (fde >= 0);
	ASSERT (fdt >= 0);
	ASSERT (fdg >= 0);
	ASSERT (fdspi >= 0);
	ASSERT (fdi2c >= 0);
	
	int vsync_counter = 0;
	int gaurd_counter = 0;
	
	struct epoll_event events [10];
	
	{
		struct itimerspec ts;
		ts.it_interval.tv_sec = 1;
		ts.it_interval.tv_nsec = 0;
		ts.it_value.tv_sec = 1;
		ts.it_value.tv_nsec = 0;
		int r = timerfd_settime (fdt, 0, &ts, NULL);
		ASSERT (r == 0);
	}
	
	//Add vsync trigger and timer trigger to list of events.
	app_epoll_add (fde, EPOLLIN | EPOLLET, fdt);
	app_epoll_add (fde, EPOLLIN | EPOLLPRI | EPOLLET, fdg);
	
	
	while (1)
	{
		//printf ("waiting for events...\n");
		int n = epoll_wait (fde, events, 10, -1);
		ASSERT (n > 0);
		//printf ("new events %i!\n", n);
		for (int i = 0; i < n; i = i + 1)
		{
			if (events [i].data.fd == fdg)
			{
				lep_epoll_gpiofd_acknowledge (fdg);
				int r = app_debug_stream (fdspi);
				if (r >= 0) {gaurd_counter = 0;}
				vsync_counter ++;
			}
			
			else if (events [i].data.fd == fdt)
			{
				lep_epoll_timerfd_acknowledge (fdt);
				printf ("vsync/sec %i\n", vsync_counter);
				vsync_counter = 0;
				if (gaurd_counter >= 3)
				{
					reboot (fdi2c);
					gaurd_counter = 0;
				}
				gaurd_counter ++;
			}
		}
	}
	
	
	return 0;
}

