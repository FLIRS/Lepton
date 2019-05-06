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



#define APP_EVENT_COUNT 10
#define APP_SAMPLE_PERIOD 10
#define APP_VSYNC_GPIOPIN 17




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

	int counter = 0;
	
	struct epoll_event events [APP_EVENT_COUNT];
	
	{
		struct itimerspec ts;
		ts.it_interval.tv_sec = APP_SAMPLE_PERIOD;
		ts.it_interval.tv_nsec = 0;
		ts.it_value.tv_sec = APP_SAMPLE_PERIOD;
		ts.it_value.tv_nsec = 0;
		int r = timerfd_settime (fdt, 0, &ts, NULL);
		ASSERT (r == 0);
	}
	
	app_epoll_add (fde, EPOLLIN | EPOLLET, fdt);
	app_epoll_add (fde, EPOLLIN | EPOLLPRI | EPOLLET, fdg);
	
	while (1)
	{
		int n = epoll_wait (fde, events, APP_EVENT_COUNT, -1);
		ASSERT (n > 0);
		for (int i = 0; i < n; i = i + 1)
		{
			if (events [i].data.fd == fdg)
			{
				lep_epoll_gpiofd_acknowledge (fdg);
				counter ++;
			}
			
			else if (events [i].data.fd == fdt)
			{
				lep_epoll_timerfd_acknowledge (fdt);
				printf ("fps %05i/%02i %05f\n", (int) counter, (int) APP_SAMPLE_PERIOD, (float) counter / APP_SAMPLE_PERIOD);
				counter = 0;
			}
		}
	}
	
	
	return 0;
}

