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


//INT_EDGE_RISING
//wiringPiISR
//wiringPiSetup
#include <wiringPi.h>

int dev;
int app_error_frames = 0;
	
void app_read_packet ()
{
	struct Lep_Packet pack [LEP2_HEIGHT];
	lep_spi_receive (dev, (uint8_t *) pack, sizeof (pack));
	
	//Sync frames
	//if (!lep_check (pack)) 
	if ((pack [0].reserved & 0x0F) == 0x0F)
	{
		//TODO: wtf is this a good solution?.
		usleep (200000);
		app_error_frames ++; 
		printf ("e : %04i\n", app_error_frames);
		return;
	}
	app_error_frames = 0;
	
	//Sync segements
	if (lep_check (pack + 20) && (pack [20].number != 20)) 
	{
		lep_spi_receive (dev, (uint8_t *) pack, LEP_PACKET_SIZE); 
		return;
	}
	
	
	for (size_t i = 0; i < LEP2_HEIGHT; i = i + 1)
	{
		//printf ("%02i : ", i);
		//app_print_byte_str ((uint8_t *)(pack + i), 10);
		//printf ("\n");
		printf ("%i ", pack [i].number);
	}
	printf ("\n");
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
   wiringPiISR (Pin, Edge, &app_read_packet);
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
	
	dev = lep_spi_open (LEP_SPI_DEV_RPI3);
	setup_wiringPi ();
	
	while (1)
	{
		sleep (1);
	}
	
	
	return 0;
}

