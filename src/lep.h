#pragma once

#include <stdbool.h> //true, false, bool
#include <sys/ioctl.h> //ioctl
#include <fcntl.h> //open
#include <endian.h> //htobe16
#include <string.h> //memset
#include <errno.h> //errno
#include <unistd.h> //read
#include <linux/spi/spidev.h> //spi_ioc_transfer
#include <linux/i2c-dev.h>//I2C_SlAVE
#include "crc.h"


#ifndef LEP_ASSERT
#define LEP_ASSERT(A, code, message, ...)
#endif


#ifndef LEP_NOTE
#define LEP_NOTE(code, message, ...)
#endif


#define LEP_BEGIN_SYSTEM_CALL (errno = 0)


enum Lep_Result
{
	LEP_SUCCESS = 0,
	LEP_ERROR_SPI = -1,
	LEP_ERROR_I2C = -2,
	LEP_INFO1 = 1
};


//Lepton FLIR Datasheet page 29.
//"The Lepton uses SPI Mode 3 (CPOL=1, CPHA=1)".
//"SCK is HIGH when idle."
#define LEP_SPI_MODE SPI_MODE_3


//Page 30.
//The maximum clock rate is 20 MHz.
//The minimum clock rate is a function of the number of bits of data per frame that need to be retrieved.
//As described in the sections that follow, the number of bits of data varies 
//depending upon user settings (video format mode, telemetry mode).
//For default conditions (Raw14 mode, telemetry disabled), 
//there are 60 video packets per frame, 
//each 1312 bits long, at approximately 25.9 frames per second.
//Therefore, the minimum rate is on the order of 2 MHz.
#define LEP_SPI_SPEED_MIN 2000000
#define LEP_SPI_SPEED_MAX 20000000
#define LEP_SPI_SPEED_RECOMENDED 16000000

#define LEP_PACKET_PER_SEC 27

//Each packet is the same size.
#define LEP_PACKET_SIZE 164

//Payload size it the size of the data in each packets.
#define LEP_PAYLOAD_SIZE 160

//TODO: Why 8 bit?
#define LEP_SPI_BITS_PER_WORD 8



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
//consequently only 16-bit transfers are allowed.
#define LEP_I2C_ADDRESS 0x2A
#define LEP_I2C_BIT_WIDTH 16
#define LEP_I2C_BYTE_WIDTH 2
#define LEP_DATAREG_COUNT 16

#define LEP_I2C_DEV_RPI3 "/dev/i2c-1"



//Register addresses.
//Page 9.
enum Lep_Reg
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


//Page 18.
//2.1.5 CCI/TWI Status Register
enum Lep_Status
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
enum Lep_Comid
{
	//4.4.12 SYS Ping Camera
	//This function sends the ping command to the camera. The camera will respond with LEP_OK if command received
	//correctly.
	LEP_COMID_PING = 0x0200,
	//Page 17.
	LEP_COMID_UPTIME  = 0x020C,
	LEP_COMID_AUXTEMP = 0x0210,
	LEP_COMID_FPATEMP = 0x0214,
	LEP_COMID_REBOOT  = 0x4842,
	LEP_COMID_GPIO    = 0x4854
};


//2.1.3.4 Command Type
//A command type specifies what the command does.
//• 0x00 Get a module property or attribute value
//• 0x01 Set a module property or attribute value
//• 0x02 Run – execute a camera operation exposed by that module
enum Lep_Comtype
{
	LEP_COMTYPE_GET = 0x00,
	LEP_COMTYPE_SET = 0x01,
	LEP_COMTYPE_RUN = 0x02,
	LEP_COMTYPE_MASK = 0x03
};


//4.6.15 OEM GPIO Mode Select
//This function gets and sets the GPIO pins mode.
enum Lep_GPIO
{
	LEP_GPIO_MODE = 0,
	LEP_GPIO_I2C_MASTER = 1,
	LEP_GPIO_SPI_MASTER_VLB_DATA = 2,
	LEP_GPIO_SPIO_MASTER_REG_DATA = 3,
	LEP_GPIO_SPI_SLAVE_VLB_DATA = 4,
	LEP_GPIO_VSYNC = 5
};


//  FLIR Lepton Datasheet Page 31.
//  16 bits                        |  16 bits       | 160 Bytes
//  ID Field                       |  CRC Field     | Payload Field
//  -------------------------------|----------------|------------------
//  4 bits      | 12 bits          |  16 bits       | 160 Bytes
//  ID_Reserved | ID_Packet_Number |  CRC           | Payload
struct __attribute__((__packed__)) Lep_Packet
{
	//The ID field is located at byte 0 .. 1.
	//The ID field is structured as following: (XXXX NNNN NNNN NNNN) 
	//where N is a packet number bit and X is reserved/secret.
	//The maximum packet number is 59 so the packet number can be 
	//extracted from (xxxx 0000 NNNN NNNN) i.e. second byte.
	//Lepton FLIR is big endian and Raspberry PI is little endian by 
	//defualt.
	uint8_t reserved;
	uint8_t number;

	//The CRC field is lcoated at bytes 2 .. 3.
	//The checksum is stored in network byte order.
	uint16_t checksum;

	//The payload field is lcoated at bytes 4 .. 164.
	union
	{
		uint8_t payload [LEP_PACKET_SIZE];
		//Lepton 2 and 3 has the same width in the packet.
		uint16_t line [LEP2_WIDTH];
	};
};


//Return false when CRC mismatch
//Return true when CRC match
bool lep_check (struct Lep_Packet * packet)
{
	//Might not be necessary.
	//This holds segment number value at packet 20.
	uint8_t preserve_reserved = packet->reserved;

	//Checksum is big endian. Convert byte order to host.
	uint16_t checksum = be16toh (packet->checksum);

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
	bool success;
	uint16_t sum = lep_crc_calculate (LEP_CRC16_CCITT, (uint8_t *) packet, sizeof (struct Lep_Packet), 0, 0);
	success = (checksum > 0 && checksum == sum);

	//Restore Checksum and Reserved.
	packet->checksum = htobe16 (checksum);
	packet->reserved = preserve_reserved;

	return success;
}


//Return -1 when not successful.
//Return count when successful.
//Receive <count> number of uint8_t <data> from SPI <device>
int lep_spi_receive (int device, uint8_t * data, size_t count)
{
	struct spi_ioc_transfer transfer;
	memset ((void *) &transfer, 0, sizeof (struct spi_ioc_transfer));

	transfer.tx_buf = (unsigned long) NULL;
	transfer.rx_buf = (unsigned long) data;
	transfer.len = count;
	transfer.delay_usecs = 0;
	transfer.speed_hz = LEP_SPI_SPEED_MAX;
	transfer.bits_per_word = LEP_SPI_BITS_PER_WORD;
	transfer.cs_change = 0;
	transfer.pad = 0;


	//memset might not be useful here.
	memset ((void *) data, 0, count);
	
	//ioctl SPI_IOC_MESSAGE returns the number of elements transfered.
	LEP_NOTE (LEP_INFO1, "SPI_IOC_MESSAGE%s", "");
	LEP_BEGIN_SYSTEM_CALL;
	int R = ioctl (device, SPI_IOC_MESSAGE (1), &transfer);
	LEP_ASSERT (R == (int) count, LEP_ERROR_SPI, "filedescriptor %i. ioctl SPI_IOC_MESSAGE", device);

	return R;
}



int lep_i2c_open (char const * name)
{
	int device;

	
	LEP_NOTE (LEP_INFO1, "open (%s, O_RDWR)", name);
	LEP_BEGIN_SYSTEM_CALL;
	device = open (name, O_RDWR);
	LEP_ASSERT (device != -1, LEP_ERROR_I2C, "open%s", "");

	if (device == -1) {return device;}
	
	int R;
	LEP_NOTE (LEP_INFO1, "ioctl (%i, I2C_SLAVE, %x)", device, LEP_I2C_ADDRESS);
	LEP_BEGIN_SYSTEM_CALL;
	R = ioctl (device, I2C_SLAVE, LEP_I2C_ADDRESS);
	LEP_ASSERT (device != -1, LEP_ERROR_I2C, "ioctl%s", "");
	
	if (R != 0) {return R;}

	return device;
}



void lep_htobe16v (uint16_t * data, size_t size8)
{
	for (size_t I = 0; I < (size8 / 2); I = I + 1)
	{
		data [I] = htobe16 (data [I]);
	}
}

void lep_be16tohv (uint16_t * data, size_t size8)
{
	for (size_t I = 0; I < (size8 / 2); I = I + 1)
	{
		data [I] = be16toh (data [I]);
	}
}

//Read.
int lep_i2c_pure_read (int device, uint16_t * data, size_t size8)
{
	int R;
	LEP_NOTE (LEP_INFO1, "read (%i, data, %i)", device, size8);
	LEP_BEGIN_SYSTEM_CALL;
	R = read (device, (void *) data, size8);
	LEP_ASSERT (R == (int) size8, LEP_ERROR_I2C, "Device %i. read", device);
	lep_be16tohv (data, size8);
	return R;
}

//Read.
int lep_i2c_pure_write (int device, uint16_t * data, size_t size8)
{
	int R;
	lep_htobe16v (data, size8);
	LEP_NOTE (LEP_INFO1, "write (%i, data, %i)", device, size8);
	LEP_BEGIN_SYSTEM_CALL;
	R = write (device, (void *) data, size8);
	LEP_ASSERT (R == (int) size8, LEP_ERROR_I2C, "Device %i. write", device);
	return R;
}


int lep_i2c_read (int dev, uint16_t reg, void * data, size_t size8)
{
	int R;
	LEP_ASSERT (size8 <= sizeof (uint16_t) * LEP_DATAREG_COUNT, LEP_ERROR_I2C, "%s", "");
	//Select start address to read from.
	R = lep_i2c_pure_write (dev, &reg, sizeof (uint16_t));
	//Read from that start address.
	R = lep_i2c_pure_read (dev, data, size8);
	return R;
}


int lep_i2c_write (int dev, uint16_t reg, void * data, size_t size8)
{
	int R;
	//The FLIR Lepton has limited amount of IO registers.
	LEP_ASSERT (size8 <= sizeof (uint16_t) * LEP_DATAREG_COUNT, LEP_ERROR_I2C, "%s", "");
	//Writing data to address location must be done in a single write.
	//Conatenate reg & data
	uint16_t buffer [LEP_DATAREG_COUNT + 1];
	//Select start address to write from.
	buffer [0] = reg;
	//Write from that start address.
	memcpy (buffer + 1, data, size8);
	R = lep_i2c_pure_write (dev, buffer, size8 + sizeof (uint16_t));
	return R;
}


//Same as (lep_i2c_read) but reads just one word.
int lep_i2c_read1 (int dev, uint16_t reg, uint16_t * data)
{
	return lep_i2c_read (dev, reg, data, sizeof (uint16_t));
}


//Same as (lep_i2c_write) but writes just one word.
int lep_i2c_write1 (int dev, uint16_t reg, uint16_t data)
{
	return lep_i2c_write (dev, reg, &data, sizeof (uint16_t));
}


int lep_i2c_com (int dev, uint16_t comid, void * data, size_t size8, uint16_t * status)
{
	int R;
	R = lep_i2c_read1 (dev, LEP_REG_STATUS, status);
	if (R <= 0) {return R;}
	if (*status & LEP_STATUS_BUSY) {return -2;};
	if (!(*status & LEP_STATUS_BOOTMODE)) {return -2;};
	if (!(*status & LEP_STATUS_BOOTSTATUS)) {return -2;};
	//Set the data length register for setting or getting values.
	R = lep_i2c_write1 (dev, LEP_REG_LENGTH, size8 / 2);
	if (R <= 0) {return R;}
	
	switch (comid & LEP_COMTYPE_MASK)
	{
		//Get a module property or attribute value
		case LEP_COMTYPE_GET:
		R = lep_i2c_write1 (dev, LEP_REG_COMMAND, comid);
		if (R <= 0) {return R;}
		R = lep_i2c_read1 (dev, LEP_REG_STATUS, status);
		if (R <= 0) {return R;}
		if (*status != LEP_STATUS_OK) {return -3;};
		R = lep_i2c_read (dev, LEP_REG_DATA0, data, size8);
		break;
		
		//Set a module property or attribute value
		case LEP_COMTYPE_SET:
		R = lep_i2c_write (dev, LEP_REG_DATA0, data, size8);
		if (R <= 0) {return R;}
		R = lep_i2c_write1 (dev, LEP_REG_COMMAND, comid);
		if (R <= 0) {return R;}
		R = lep_i2c_read1 (dev, LEP_REG_STATUS, status);
		if (R <= 0) {return R;}
		if (*status != LEP_STATUS_OK) {return -4;};
		break;
		
		//Run – execute a camera operation exposed by that module
		case LEP_COMTYPE_RUN:
		R = lep_i2c_write1 (dev, LEP_REG_COMMAND, comid);
		if (R <= 0) {return R;}
		R = lep_i2c_read1 (dev, LEP_REG_STATUS, status);
		if (R <= 0) {return R;}
		if (*status != LEP_STATUS_OK) {return -5;};
		break;
	}
	return R;
}





