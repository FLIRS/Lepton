#define LEP_ASSERT_CF(A,C,F,...) ASSERT_CF(A,C,F,__VA_ARGS__)
//#define LEP_NOTE(C,F,...) note(C,F,__VA_ARGS__)

#include "debug.h"
#include "lep.h"
#include "crc.h"
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
	int C = 1;
	while (C != -1)
	{
		C = getopt (argc, argv, "");
		switch (C)
		{	
			default:
			break;
		}
	}
	
	int efd;
	int tfd;
	int pinfd;
	int vsync_counter = 0;
	int gaurd_counter = 0;
	int dev_spi = lep_spi_open (LEP_SPI_DEV_RPI3);
	int dev_i2c = lep_i2c_open (LEP_I2C_DEV_RPI3);
	
	struct epoll_event events [10];
	
	tfd = timerfd_create (CLOCK_MONOTONIC, 0);
	ASSERT (tfd > 0);
	
	{
		struct itimerspec ts;
		ts.it_interval.tv_sec = 1;
		ts.it_interval.tv_nsec = 0;
		ts.it_value.tv_sec = 1;
		ts.it_value.tv_nsec = 0;
		int r = timerfd_settime (tfd, 0, &ts, NULL);
		ASSERT (r == 0);
	}
	
	pinfd = lep_isr_init (17);
	printf ("pinfd %i\n", pinfd);
	efd = epoll_create1 (0);
	ASSERT_F (efd > 0, "%s", "");
	
	//Add vsync trigger and timer trigger to list of events.
	app_epoll_add (efd, EPOLLIN | EPOLLET, tfd);
	app_epoll_add (efd, EPOLLIN | EPOLLPRI | EPOLLET, pinfd);
	
	
	while (1)
	{
		//printf ("waiting for events...\n");
		int n = epoll_wait (efd, events, 10, -1);
		ASSERT (n > 0);
		//printf ("new events %i!\n", n);
		for (int i = 0; i < n; i = i + 1)
		{
			if (events [i].data.fd == pinfd)
			{
				int r = app_debug_stream (dev_spi);
				if (r >= 0) {gaurd_counter = 0;}
				app_epoll_handle_gpio (pinfd);
				vsync_counter ++;
			}
			
			else if (events [i].data.fd == tfd)
			{
				app_epoll_handle_timer (tfd);
				printf ("vsync/sec %i\n", vsync_counter);
				vsync_counter = 0;
				if (gaurd_counter >= 3)
				{
					app_reboot (dev_i2c);
					gaurd_counter = 0;
				}
				gaurd_counter ++;
			}
		}
	}
	
	
	return 0;
}

