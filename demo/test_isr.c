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
	struct epoll_event events [10];
	int pinfd;
	
	pinfd = lep_isr_init (17);
	printf ("pinfd %i\n", pinfd);
	efd = epoll_create1 (0);
	ASSERT_F (efd > 0, "%s", "");
	
	app_epoll_add (efd, EPOLLIN | EPOLLPRI | EPOLLET, pinfd);
	
	while (1)
	{
		printf ("waiting for events...\n");
		int n = epoll_wait (efd, events, 10, -1);
		ASSERT_F (n > 0, "%s", "");
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

