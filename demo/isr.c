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
	struct epoll_event events [10];
	int pinfd;
	
	pinfd = lep_isr_init (17);
	printf ("pinfd %i\n", pinfd);
	efd = epoll_create1 (0);
	ASSERT_ACF (efd > 0, 0, "%s", "");
	
	{
		struct epoll_event ev;
		ev.events = EPOLLIN | EPOLLPRI | EPOLLET;
		ev.data.fd = pinfd;
		int r = epoll_ctl (efd, EPOLL_CTL_ADD, pinfd, &ev);
		ASSERT_ACF (r == 0, 0, "%s", "");
	}
	
	while (1)
	{
		printf ("waiting for events...\n");
		int n = epoll_wait (efd, events, 10, -1);
		ASSERT_ACF (n > 0, 0, "%s", "");
		printf ("new events %i!\n", n);
		for (int i = 0; i < n; i = i + 1)
		{
			char buf [10];
			int res = read (pinfd, buf, sizeof (buf));
			lseek (pinfd, 0, SEEK_SET);
			printf ("event res %i\n", res);
		}
	}
	
	
	return 0;
}

