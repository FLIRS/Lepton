#pragma once

#include <asm/byteorder.h>
#include <linux/build_bug.h>
#include "lepcrc.h"

//Assert is optional.
//Used for debugging.
//All possable outcomes should be asserted.
#ifndef LEP_ASSERT
#define LEP_ASSERT(A)
#endif

#ifndef LEP_ASSERT_C
#define LEP_ASSERT_C(A, C)
#endif

#ifndef LEP_ASSERT_F
#define LEP_ASSERT_F(A, F, ...)
#endif

#ifndef LEP_ASSERT_CF
#define LEP_ASSERT_CF(A, C, F, ...)
#endif


//Trace is optional.
//Used for debugging.
#ifndef LEP_TRACE
#define LEP_TRACE(F)
#endif

#ifndef LEP_TRACE_F
#define LEP_TRACE_F(F, ...)
#endif

#ifndef LEP_TRACE_CF
#define LEP_TRACE_CF(C, F, ...)
#endif


//Unofficial result
enum lep_result
{
	LEP_SUCCESS = 0,
	LEP_SEG0 = 1,
	LEP_NOTREADY = 2,
	
	LEP_ERROR = -1,
	LEP_ERROR_NULL = -2,
	LEP_ERROR_ARG = -3,
	LEP_ERROR_RANGE = -4,
	LEP_ERROR_CRC = -5,
	LEP_ERROR_NOT20 = -6,
	LEP_ERROR_OPEN = -10,
	LEP_ERROR_WRITE = -11,
	LEP_ERROR_SPI = -11,
	LEP_ERROR_I2C = -12
};


//9.2.1 VoSPI Physical Interface
//The MOSI (Master Out/Slave In) signal is not currently employed and should be connected to a logic low.
//Implementations are restricted to a single master and single slave. The Lepton 3 uses SPI Mode 3 (CPOL=1,
//CPHA=1); SCK is HIGH when idle. Data is set up by the Lepton 3 on the falling edge of SCK and should be
//sampled by the host controller on the rising edge.
#define LEP_SPI_MODE SPI_MODE_3


//9.2.1 VoSPI Physical Interface
//The maximum SPI clock rate is 20 MHz. The minimum clock rate is a function of the number of bits of data per
//frame that need to be retrieved. As described in the sections that follow, the number of bits of data varies
//depending upon user settings (video format mode, telemetry mode). For default conditions (Raw14 mode,
//telemetry disabled), there are 60 video packets per segment, each 1312 bits long, 4 segments per frame, and
//approximately 26.4 frames per second. Therefore, the minimum rate is on the order of 8.3 MHz.
#define LEP_SPI_SPEED_MIN 2000000
#define LEP_SPI_SPEED_MAX 20000000
#define LEP_SPI_SPEED_RECOMENDED 16000000


//Not exactly
#define LEP2_PACKET_PER_SEC 27
#define LEP3_PACKET_PER_SEC 108


//Each packet is the same size.
#define LEP_PACKET_SIZE 164


//Payload size it the size of the data in each packets.
#define LEP_PAYLOAD_SIZE 160


//Segment size it the size of a entire frame.
#define LEP_SEGMENT_SIZE (LEP_PACKET_SIZE * LEP2_HEIGHT)


//TODO: Is there a reason for using 8 bit per word?
#define LEP_SPI_BITS_PER_WORD 8


//2.1.2.1 Reading from the camera
//Reading DATA from the camera using the CCI/TWI interfaces follows the I2C standard except the DATA is
//all 16-bit wide.
#define LEP_REG_SIZE 16


//Width and hight by pixels.
#define LEP2_WIDTH 80
#define LEP2_HEIGHT 60
#define LEP2_NANOSEC_PER_PACKET (1000000000/27)

//Width and hight by pixels.
#define LEP3_WIDTH 160
#define LEP3_HEIGHT 120
#define LEP3_NANOSEC_PER_PACKET (1000000000/106)


//2.1.2 CCI/TWI Interface
//The CCI/TWI interface is similar to the I2C standard; however, Lepton registers are all 16-bits wide and
//consequently only 16-bit transfers are allowed. This is illustrated in Figure 5. Device parameters are
//listed in Table 1.
#define LEP_I2C_ADDRESS 0x2A
#define LEP_I2C_BIT_WIDTH 16
#define LEP_I2C_BYTE_WIDTH 2
#define LEP_DATAREG_COUNT 16

#define LEP_I2C_DEV_RPI3 "/dev/i2c-1"
#define LEP_SPI_DEV_RPI3 "/dev/spidev0.0"


//2.1 CCI/TWI Register Protocol
//The Lepton camera module supports a command and control interface (CCI) hosted on a Two-Wire
//Interface (TWI) similar to I2C. The interface consists of a small number of registers through which a Host
//issues commands to, and retrieves responses from the Lepton camera module. See Figure 1.
//Figure 1 Lepton CCI/TWI Registers
enum lep_reg
{
	LEP_REG_STATUS = 0x0002,
	LEP_REG_COMMAND = 0x0004,
	LEP_REG_LENGTH = 0x0006,
	LEP_REG_DATA0 = 0x0008,
	LEP_REG_DATA1 = 0x000A,
	LEP_REG_DATA2 = 0x000C,
	LEP_REG_DATA3 = 0x000E,
	LEP_REG_DATA4 = 0x0010,
	LEP_REG_DATA5 = 0x0012,
	LEP_REG_DATA6 = 0x0014,
	LEP_REG_DATA7 = 0x0016,
	LEP_REG_DATA8 = 0x0018,
	LEP_REG_DATA9 = 0x001A,
	LEP_REG_DATA10 = 0x001C,
	LEP_REG_DATA11 = 0x001E,
	LEP_REG_DATA12 = 0x0020,
	LEP_REG_DATA13 = 0x0022,
	LEP_REG_DATA14 = 0x0024,
	LEP_REG_DATA15 = 0x0026
};


//2.1.5 CCI/TWI Status Register
//The Status register, located at Register Address 0x0002 and illustrated in Figure 11, is used to
//communicate command status and camera boot status. Whenever a Host issues a command to the
//camera by writing to the Command Register, the camera automatically asserts (sets to 1) the command
//BUSY bit (Bit 0) in the Status register. When the command is completed, the response code is written
//into the upper 8-bits of the Status register (Bits 15-8). Then the camera de-asserts (sets to 0) the BUSY
//bit to signal the Host the command is complete. See Figure 12 for the possible responses from the
//camera to a command.
enum lep_status
{
	LEP_STATUS_BUSY = 0x0001,
	LEP_STATUS_BOOTMODE = 0x0002,
	LEP_STATUS_BOOTSTATUS = 0x0004,
	LEP_STATUS_OK = (LEP_STATUS_BOOTMODE | LEP_STATUS_BOOTSTATUS),
	LEP_STATUS_ERROR_MASK = 0xFF00,
	LEP_STATUS_RESERVED_MASK = 0x00F8
};



//2.1.3.3 Command ID
//For each of the Lepton camera modules, a unique Lepton Command ID identifies an element of the
//module, either an attribute or property, or an action. Each camera module exposes up to 64 Command
//IDs assigned to attributes and/or methods of that module.	
enum lep_comid
{
//4.4.12 SYS Ping Camera
//This function sends the ping command to the camera. The camera will respond with LEP_OK if command received
//correctly.
LEP_COMID_PING           = 0x0200,

//4.4.15 SYS Camera Uptime
//This command returns the Lepton Camera’s current uptime in milliseconds. The uptime is the time since the
//camera was brought out of Standby. The uptime counter is implemented as a 32-bit counter and as such will roll-
//over after the maximum count of 0xFFFFFFFF (1193 hours) is reached and restart at 0x00000000.
LEP_COMID_UPTIME         = 0x020C,

//4.4.16 SYS AUX Temperature Kelvin
//This command returns the Lepton Camera’s AUX Temperature in Kelvin.
LEP_COMID_AUXTEMP        = 0x0210,

//4.4.17 SYS FPA Temperature Kelvin
//This command returns the Lepton Camera’s FPA Temperature in Kelvin.
LEP_COMID_FPATEMP        = 0x0214,

//4.6.11 OEM Run Camera Re-Boot
//This function commands the Camera to re-boot. The Camera is first shutdown, and then restarts automatically.
LEP_COMID_REBOOT         = 0x4842,

//4.6.15 OEM GPIO Mode Select
//This function gets and sets the GPIO pins mode.
LEP_COMID_GPIO           = 0x4854,

//4.6.16 OEM GPIO VSync Phase Delay
//This function gets and sets the GPIO VSync phase delay. The Lepton Camera can issue a pulse on GPIO3 when
//there is an inter VSync. The output pulse may be issued in phase with the camera’s internal VSync, or it may be
//issued earlier or later. This command controls this phase relationship. The delays are in line periods,
//approximately 0.5 milliseconds per period. The phase delay is limited to +/- 3 line periods.
LEP_COMID_VSYNC_DELAY    = 0x4858
};


//2.1.3.4 Command Type
//A command type specifies what the command does.
//• 0x00 Get a module property or attribute value
//• 0x01 Set a module property or attribute value
//• 0x02 Run – execute a camera operation exposed by that module
enum lep_comtype
{
	LEP_COMTYPE_GET = 0x00,
	LEP_COMTYPE_SET = 0x01,
	LEP_COMTYPE_RUN = 0x02,
	LEP_COMTYPE_MASK = 0x03
};


//4.6.15 OEM GPIO Mode Select
//This function gets and sets the GPIO pins mode.
enum lep_gpio
{
	LEP_GPIO_MODE = 0,
	LEP_GPIO_I2C_MASTER = 1,
	LEP_GPIO_SPI_MASTER_VLB_DATA = 2,
	LEP_GPIO_SPIO_MASTER_REG_DATA = 3,
	LEP_GPIO_SPI_SLAVE_VLB_DATA = 4,
	LEP_GPIO_VSYNC = 5
};


//4.6.16 OEM GPIO VSync Phase Delay
//This function gets and sets the GPIO VSync phase delay. The Lepton Camera can issue a pulse on GPIO3 when
//there is an inter VSync. The output pulse may be issued in phase with the camera’s internal VSync, or it may be
//issued earlier or later. This command controls this phase relationship. The delays are in line periods,
//approximately 0.5 milliseconds per period. The phase delay is limited to +/- 3 line periods.
enum lep_vsyncdelay
{
	LEP_VSYNCDELAY_MINUS3 = -3,
	LEP_VSYNCDELAY_MINUS2 = -2,
	LEP_VSYNCDELAY_MINUS1 = -1,
	LEP_VSYNCDELAY_NONE = 0,
	LEP_VSYNCDELAY_PLUS1 = 1,
	LEP_VSYNCDELAY_PLUS2 = 2,
	LEP_VSYNCDELAY_PLUS3 = 3
};


//9.2.2.1 VoSPI Packets
//As depicted in Figure 21, each packet contains a 4-byte header followed by either a 160-byte or 240-byte
//payload. Note: because the payload size differs between video formats, the setting should be selected before
//VoSPI synchronization is established. If the setting is changed while VoSPI is active, it is necessary to
//re-synchronize (see VoSPI Stream, page 43).
//For video packets, the header includes a 2-byte ID and a 2-byte CRC. The ID field encodes the segment
//number (1, 2, 3, or 4) and the packet number required to determine where the packet belongs in relation to the
//final 160 x 120 image (or 160x122 if telemetry is enabled). The segment and packet location in each frame is
//exemplified in Figure 22. Recall that with telemetry disabled, each segment is comprised of 60 packets, each
//containing pixel data for half of a video line. With telemetry enabled, each segment is comprised of 61 packets.
//As shown in Figure 23, the first bit of the ID field is always a zero. The next three bits are referred to as the TTT
//bits, and the following 12 are the packet number. Note that packet numbers restart at 0 on each new segment.
//For all but packet number 20, the TTT bits can be ignored. On packet 20, the TTT bits encode the segment
//number (1, 2, 3, or 4). The encoded segment number can also have a value of zero. In this case the entire
//segment is invalid data and should be discarded. Figure 23 also shows an example of Packet 20 of Segment 3.
//  16 bits                        |  16 bits       | 160 Bytes
//  ID Field                       |  CRC Field     | Payload Field
//  -------------------------------|----------------|------------------
//  4 bits      | 12 bits          |  16 bits       | 160 Bytes
//  ID_Reserved | ID_Packet_Number |  CRC           | Payload
__attribute__((__packed__)) 
struct lep_packet
{
	//The ID field is located at byte 0 .. 1.
	//The ID field is structured as following: (XXXX NNNN NNNN NNNN) 
	//where N is a packet number bit and X is reserved/secret.
	//The maximum packet number is 59 so the packet number can be 
	//extracted from (xxxx 0000 NNNN NNNN) i.e. second byte.
	//Lepton FLIR is big endian and Raspberry PI is little endian by 
	//defualt.
	u8 reserved;
	u8 number;

	//The CRC field is lcoated at bytes 2 .. 3.
	//The checksum is stored in network byte order.
	u16 checksum;

	//The payload field is lcoated at bytes 4 .. 164.
	union
	{
		u8 payload [LEP_PAYLOAD_SIZE];
		//Lepton 2 and 3 has the same width in the packet.
		u16 line [LEP2_WIDTH];
	};
};


//static_assert (sizeof (struct lep_packet) == LEP_PACKET_SIZE, "The struct is not correct size");
//static_assert ((sizeof (struct lep_packet) * LEP2_HEIGHT) == LEP_SEGMENT_SIZE, "The struct is not correct size");


//Return false when CRC mismatch
//Return true when CRC match
int lep_check (struct lep_packet * packet)
{
	int success;
	//Might not be necessary.
	//This holds segment number value at packet 20.
	u8 preserve_reserved = packet->reserved;

	//Checksum is big endian. Convert byte order to host.
	u16 checksum = __be16_to_cpu (packet->checksum);
	u16 sum;
	
	//FLIR Lepton Datasheet Page 31.
	//The four most-significant bits of the ID are set to zero 
	//for calculation of the CRC.
	packet->reserved = packet->reserved & 0x0F;

	//FLIR Lepton Datasheet Page 31.
	//All bytes of the CRC are set to zero for calculation the CRC.
	packet->checksum = 0;

	//FLIR Lepton Datasheet Page 31.
	//CRC16_CCITT: x^16 + x^12 + x^5 + x^0
	//Undocumented: CRC Seed equal zero.
	//Checksum > 0 might not be useful here.
	sum = lepcrc ((u8 *) packet, sizeof (struct lep_packet), 0, 0);
	success = (checksum > 0 && checksum == sum);

	//Restore Checksum and Reserved.
	packet->checksum = __cpu_to_be16 (checksum);
	packet->reserved = preserve_reserved;

	return success;
}


int lep_mismatchv (struct lep_packet packet [], size_t n)
{
	while (n--)
	{
		if (lep_check (packet + n) == false) {return (int)n;}
	}
	return -1;
}


void lep_htobe16v (u16 data [], size_t n)
{
	while (n--)
	{
		data [n] = __cpu_to_be16 (data [n]);
	}
}


void lep_be16tohv (u16 data [], size_t n)
{
	while (n--)
	{
		data [n] = __be16_to_cpu (data [n]);
	}
}

