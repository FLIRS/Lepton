#pragma once

#include <sys/epoll.h>
#include <stdio.h>
#include <stdint.h>
#include "debug.h"
#include "tcol.h"
#include "lep.h"

int app_epoll_add (int efd, uint32_t events, int fd)
{
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = fd;
	int r = epoll_ctl (efd, EPOLL_CTL_ADD, fd, &ev);
	ASSERT_F (r == 0, "%s", "");
	return r;
}


void app_print_byte_str (uint8_t * x, size_t count)
{
	for (size_t i = 0; i < count; i = i + 1)
	{
		printf ("%02x ", x [i]);
	}
}


int app_debug_stream (int fd)
{
	struct lep_packet pack [LEP2_HEIGHT];
	int seg = lep3_read_segment (fd, pack);
	if (seg < 0) {return seg;}
	for (size_t i = 0; i < LEP2_HEIGHT; i = i + 1)
	{
		uint8_t x = pack [i].number;
		char const * c = TCOL (TCOL_BOLD, TCOL_RED, TCOL_DEFAULT);
		if (lep_check (pack + i)) 
		{
			c = TCOL (TCOL_BOLD, TCOL_GREEN, TCOL_DEFAULT);
			if (i == 20)
			{
				c = TCOL (TCOL_BOLD, TCOL_BLUE, TCOL_DEFAULT);
				x = pack [i].reserved >> 4;
			}
		}
		printf ("%s", c);
		printf ("%02x ", x);
		printf ("%s", TCOL_RESET);
	}
	printf ("\n");
	
	return LEP_SUCCESS;
}

