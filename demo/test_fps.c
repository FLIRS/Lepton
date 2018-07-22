#define LEP_ASSERT ASSERT
#define LEP_ASSERT_C ASSERT_C
#define LEP_ASSERT_F ASSERT_F
#define LEP_ASSERT_CF ASSERT_CF
#define LEP_TRACE TRACE
#define LEP_TRACE_C TRACE_C
#define LEP_TRACE_CF TRACE_CF

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



#define APP_EVENT_COUNT 10
#define APP_SAMPLE_PERIOD 10
#define APP_VSYNC_GPIOPIN 17




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
	int counter = 0;
	
	struct epoll_event events [APP_EVENT_COUNT];
	
	tfd = timerfd_create (CLOCK_MONOTONIC, 0);
	ASSERT_F (tfd > 0, "%s", "");
	
	{
		struct itimerspec ts;
		ts.it_interval.tv_sec = APP_SAMPLE_PERIOD;
		ts.it_interval.tv_nsec = 0;
		ts.it_value.tv_sec = APP_SAMPLE_PERIOD;
		ts.it_value.tv_nsec = 0;
		int r = timerfd_settime (tfd, 0, &ts, NULL);
		ASSERT_F (r == 0, "%s", "");
	}
	
	pinfd = lep_isr_init (APP_VSYNC_GPIOPIN);
	printf ("pinfd %i\n", pinfd);
	efd = epoll_create1 (0);
	ASSERT_F (efd > 0, "%s", "");
	
	app_epoll_add (efd, EPOLLIN | EPOLLET, tfd);
	app_epoll_add (efd, EPOLLIN | EPOLLPRI | EPOLLET, pinfd);
	
	
	while (1)
	{
		int n = epoll_wait (efd, events, APP_EVENT_COUNT, -1);
		ASSERT_F (n > 0, "%s", "");
		for (int i = 0; i < n; i = i + 1)
		{
			if (events [i].data.fd == pinfd)
			{
				char c;
				lseek (pinfd, 0, SEEK_SET);
				int res = read (pinfd, &c, 1);
				ASSERT_CF (res == 1, 0, "%s", "");
				counter ++;
			}
			
			else if (events [i].data.fd == tfd)
			{
				uint64_t m;
				int res = read (tfd, &m, sizeof (m));
				ASSERT_F (res == sizeof (m), "%s", "");
				printf ("F*%i: %i\n", (int) counter, (int) APP_SAMPLE_PERIOD);
				printf ("   F: %f\n", (float) counter / APP_SAMPLE_PERIOD);
				counter = 0;
			}
		}
	}
	
	
	return 0;
}

