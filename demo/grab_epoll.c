//#define LEP_ASSERT ASSERT
//#define LEP_ASSERT_C ASSERT_C
//#define LEP_ASSERT_F ASSERT_F
//#define LEP_ASSERT_CF ASSERT_CF

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
				int r = lep3_stream (fdspi, STDOUT_FILENO);
				if (r >= 0) {gaurd_counter = 0;}
			}
			
			else if (events [i].data.fd == fdt)
			{
				lep_epoll_timerfd_acknowledge (fdt);
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

