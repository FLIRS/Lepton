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
	int8_t error;
	//error = (status >> 8) | 0xFF00;
	error = status >> 8;
	char * ci = TCOL (TCOL_BOLD, TCOL_YELLOW, TCOL_DEFAULT);
	char * ce = NULL;
	if (error < 0) {ce = TCOL (TCOL_BOLD, TCOL_RED, TCOL_DEFAULT);}
	else {ce = TCOL (TCOL_BOLD, TCOL_GREEN, TCOL_DEFAULT);}
	printf ("%s%30s%s : %i\n", ci, "Status", TCOL_RESET, (int)status);
	printf ("%s%30s%s : %s%i\n", ci, "status >> 8 = Error", TCOL_RESET, ce, (int)error);
	printf ("%s%30s%s : %i\n", ci, "LEP_STATUS_BUSY", TCOL_RESET, (int)!!(status & LEP_STATUS_BUSY));
	printf ("%s%30s%s : %i\n", ci, "LEP_STATUS_BOOTMODE", TCOL_RESET, (int)!!(status & LEP_STATUS_BOOTMODE));
	printf ("%s%30s%s : %i\n", ci, "LEP_STATUS_BOOTSTATUS", TCOL_RESET, (int)!!(status & LEP_STATUS_BOOTSTATUS));
}


uint16_t app_status (int dev)
{
	uint16_t status = 0;
	lep_i2c_read1 (dev, LEP_REG_STATUS, &status);
	return status;
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


int app_read_stream (int dev, struct Lep_Packet * pack)
{
	lep_spi_receive (dev, (uint8_t *) pack, LEP_SEGMENT_SIZE);
	
	for (size_t i = 0; i < LEP2_HEIGHT; i = i + 1)
	{
		if (lep_check (pack + i) == 0)
		{
			return -1;
		}
	}
	
	if (pack [20].number != 20) 
	{
		lep_spi_receive (dev, (uint8_t *) pack, LEP_PACKET_SIZE);
		return -2;
	}
	
	//reserved has segment number in packet 20.
	return (pack [20].reserved >> 4);
}


int app_debug_stream (int dev)
{
	struct Lep_Packet pack [LEP2_HEIGHT];
	
	int r = app_read_stream (dev, pack);
	if (r < 0)
	{
		return r;
	}
	
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
	
	return r;
}


void app_epoll_handle_gpio (int fd)
{
	lseek (fd, 0, SEEK_SET);
	char c;
	int r = read (fd, &c, 1);
	ASSERT (r == 1);
}


void app_epoll_handle_timer (int fd)
{
	uint64_t m;
	int r = read (fd, &m, sizeof (m));
	ASSERT (r == sizeof (m));
}
