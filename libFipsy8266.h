#ifndef __LIBFIPSY8266_H_
#define __LIBFIPSY8266_H_

#include<SPI.h>
#include<MachXO2.h>

typedef volatile uint32_t PortReg;
typedef uint32_t PortMask;

class fipsy8266Class {
	public:
		fipsy8266Class(uint8_t csPin = 15, SPIClass *useSPI = &SPI);
		void begin();
		//Returns 0xffffffff on error, otherwise the ID
		uint32_t readID();
		//Returns -1 on error, 0 on success
		int8_t erase();
	private:
		void open();
		void spiTransaction(int Count);
		
		uint8_t _csPin;
		SPIClass *_SPI;
		volatile PortReg *_csPort;
		PortMask _csMask;
		uint8_t SPIBuf[100];

		/*
		Count of bytes in the SPI buffer
		This is filled based on the count to send in a transaction
		On return from a transaction, this will specify the number of bytes returned
		bytes returned is the entire SPI transaction, not just the data, so the value
		should not change unless something went wrong.
		*/
		uint8_t MachXO2_Count = -1;
 };

#endif