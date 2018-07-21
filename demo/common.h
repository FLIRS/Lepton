#pragma once

#include <sys/epoll.h>
#include "debug.h"

int app_epoll_add (int efd, uint32_t events, int fd)
{
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = fd;
	int r = epoll_ctl (efd, EPOLL_CTL_ADD, fd, &ev);
	ASSERT_F (r == 0, "%s", "");
	return r;
}
