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


//INT_EDGE_RISING
//wiringPiISR
//wiringPiSetup
#include <wiringPi.h>

int dev_spi;
int dev_i2c;
int gaurd_counter = 0;
	
void wpi_isr ()
{
	int r = app_debug_stream (dev_spi);
	if (r >= 0) {gaurd_counter = 0;}
}


//wiringPi handling the interrupt of GPIO3 vsync of FLIR Lepton.
void setup_wiringPi ()
{
   int Result;
   Result = wiringPiSetup ();
   ASSERT_F (Result >= 0, "wiringPiSetup returned %i", Result);
   piHiPri (99);
   int Pin = 0;
   int Edge = INT_EDGE_RISING;
   wiringPiISR (Pin, Edge, &wpi_isr);
}




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
	
	dev_spi = lep_spi_open (LEP_SPI_DEV_RPI3);
	dev_i2c = lep_i2c_open (LEP_I2C_DEV_RPI3);
	
	setup_wiringPi ();
	
	while (1)
	{
		sleep (1);
		if (gaurd_counter >= 3)
		{
			app_reboot (dev_i2c);
		}
		gaurd_counter ++;
	}
	
	
	return 0;
}

