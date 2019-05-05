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


void app_print_status (uint16_t status)
{
	//TODO: Should error be 8 bit?
	int error;
	//error = (status >> 8) | 0xFF00;
	error = status >> 8;
	char const * ci = TCOL (TCOL_BOLD, TCOL_YELLOW, TCOL_DEFAULT);
	char const * ce = NULL;
	if (error < 0) {ce = TCOL (TCOL_BOLD, TCOL_RED, TCOL_DEFAULT);}
	else {ce = TCOL (TCOL_BOLD, TCOL_GREEN, TCOL_DEFAULT);}
	printf ("%s%30s%s : %i\n", ci, "Status", TCOL_RESET, (int)status);
	printf ("%s%30s%s : %s%i\n", ci, "status >> 8 = Error", TCOL_RESET, ce, (int)error);
	printf ("%s%30s%s : %i\n", ci, "LEP_STATUS_BUSY", TCOL_RESET, (int)!!(status & LEP_STATUS_BUSY));
	printf ("%s%30s%s : %i\n", ci, "LEP_STATUS_BOOTMODE", TCOL_RESET, (int)!!(status & LEP_STATUS_BOOTMODE));
	printf ("%s%30s%s : %i\n", ci, "LEP_STATUS_BOOTSTATUS", TCOL_RESET, (int)!!(status & LEP_STATUS_BOOTSTATUS));
}


void app_reboot (int dev)
{
	uint16_t status = 0;
	//int dev = lep_i2c_open (LEP_I2C_DEV_RPI3);
	printf ("dev: %i\n", dev);
	printf ("rebooting..");
	lep_i2c_com (dev, LEP_COMID_REBOOT | LEP_COMTYPE_RUN, NULL, 0, &status);
	sleep (2);
	uint16_t mode = LEP_GPIO_VSYNC;
	lep_i2c_com (dev, LEP_COMID_GPIO | LEP_COMTYPE_SET, &mode, sizeof (uint16_t), &status);
	app_print_status (status);
	//close (dev);
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

