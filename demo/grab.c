#define LEP_ASSERT_ACF(A,C,F,...) ASSERT_ACF(A,C,F,__VA_ARGS__)
//#define LEP_NOTE(C,F,...) note(C,F,__VA_ARGS__)

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


#include <sys/epoll.h>



int app_epoll_add (int efd, uint32_t events, int fd)
{
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = fd;
	int r = epoll_ctl (efd, EPOLL_CTL_ADD, fd, &ev);
	ASSERT_ACF (r == 0, 0, "%s", "");
	return r;
}


int main (int argc, char * argv [])
{
	int efd;
	int tfd;
	int pinfd;
	int counter = 0;
	
	struct epoll_event events [10];
	
	tfd = timerfd_create (CLOCK_MONOTONIC, 0);
	ASSERT_ACF (tfd > 0, 0, "%s", "");
	
	{
		struct itimerspec ts;
		ts.it_interval.tv_sec = 1;
		ts.it_interval.tv_nsec = 0;
		ts.it_value.tv_sec = 1;
		ts.it_value.tv_nsec = 0;
		int r = timerfd_settime (tfd, 0, &ts, NULL);
		ASSERT_ACF (r == 0, 0, "%s", "");
	}
	
	pinfd = lep_isr_init (17);
	printf ("pinfd %i\n", pinfd);
	efd = epoll_create1 (0);
	ASSERT_ACF (efd > 0, 0, "%s", "");
	
	app_epoll_add (efd, EPOLLIN | EPOLLET, tfd);
	app_epoll_add (efd, EPOLLIN | EPOLLPRI | EPOLLET, pinfd);
	
	
	while (1)
	{
		//printf ("waiting for events...\n");
		int n = epoll_wait (efd, events, 10, -1);
		ASSERT_ACF (n > 0, 0, "%s", "");
		//printf ("new events %i!\n", n);
		for (int i = 0; i < n; i = i + 1)
		{
			if (events [i].data.fd == pinfd)
			{
				char buf [10];
				lseek (pinfd, 0, SEEK_SET);
				int res = read (pinfd, buf, sizeof (buf));
				//if (buf [0] == '1')
				{
					counter ++;
				}
				//write (STDOUT_FILENO, buf, res);
				//printf ("event res %i\n", res);
				//printf ("event res %s\n", buf);
			}
			
			else if (events [i].data.fd == tfd)
			{
				uint64_t m;
				int res = read (tfd, &m, sizeof (m));
				printf ("tfd %i\n", counter);
				counter = 0;
			}
		}
	}
	
	
	return 0;
}

