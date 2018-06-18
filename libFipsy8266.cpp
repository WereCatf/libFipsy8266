#include<libFipsy8266.h>

#define MachXO2_Command		SPIBuf[0]
#define pMachXO2_Operand	(&SPIBuf[1])
#define pMachXO2_Data		(&SPIBuf[4])
#define SPIBUFINIT			{ MachXO2_Count = 0; memcpy_P(SPIBuf, SPIBUF_DEFAULT, 20); }
#define MachXO2_SPITrans(c)	{ MachXO2_Count = c; spiTransaction(MachXO2_Count); }   
#define csHigh				{ *_csPort |= _csMask; }
#define csLow				{ *_csPort &= ~_csMask; }

uint8_t SPIBUF_DEFAULT[20] PROGMEM = { 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
SPISettings spiSettings(400000, MSBFIRST, SPI_MODE0);

fipsy8266Class::fipsy8266Class(uint8_t csPin, SPIClass *useSPI) {
	_csPin = csPin;
	_SPI = useSPI;
}

void fipsy8266Class::begin(){
	_csPort    = portOutputRegister(digitalPinToPort(_csPin));
	_csMask = digitalPinToBitMask(_csPin);

	pinMode(_csPin, OUTPUT);
	csHigh;
	_SPI->begin();
	open();
}

void fipsy8266Class::open(void)
{
	// Send a NOP to wakeup the device
	SPIBUFINIT;
	MachXO2_Command = MACHXO2_CMD_NOP;
	pMachXO2_Operand[0] = 0xFF;
	pMachXO2_Operand[1] = 0xFF;
	pMachXO2_Operand[2] = 0xFF;
	MachXO2_SPITrans(4);
}

/*
	SPI_TRANSACTION completes the data transfer to and/or from the device 
	per the methods required by this system.  This uses the global defined
	SPI port handle, which is assumed to be open if this call is reached 
	from a routine in this code.  It is also assumed that the arguments are
	valid based on the controlled nature of calls to this routine.
*/
void fipsy8266Class::spiTransaction(int Count)
{
	int i;
	csLow;

	_SPI->beginTransaction(spiSettings);
	for(i = 0; i < Count; i++) SPIBuf[i] = _SPI->transfer(SPIBuf[i]);
	_SPI->endTransaction();

	csHigh;
}

/*
	FIPSY_READDEVICEID retrieves the device identification number from the
	FPGA connected to the SPI port.  This number can be used to verify that
	the SPI is working and we are talking to the right device.  To improve
	future flexibility, this routine does not decide if this is actually
	the right device, but just returns the four bytes it got.
*/
uint32_t fipsy8266Class::readID()
{ 
	uint32_t id;
	
	//Wake the device up, just in case
	open();
	// Construct the command  
	SPIBUFINIT;
	MachXO2_Command = MACHXO2_CMD_READ_DEVICEID;

	MachXO2_SPITrans(8);

	// Get the data
	id = SPIBuf[4] + (SPIBuf[5] << 8) + (SPIBuf[6] << 16) + (SPIBuf[7] << 24);

	// If SPI isn't connected, we might get either a 0x0 or 0xffffffff;
	// make sure we always return 0xffffffff, then.
	if(id == 0) return 0xffffffff;
	return id;
}

int8_t fipsy8266Class::erase()
{
	uint8_t busy = 0x80;
	uint32_t timeout = 0;

	// Send command to enter programming mode
	// We use only offline mode
	SPIBUFINIT;
	MachXO2_Command = MACHXO2_CMD_ENABLE_OFFLINE;
	pMachXO2_Operand[0] = 0x08;
	MachXO2_SPITrans(4);

	// Apparently this mode change takes a brief moment, ok to just wait it out here
	delay(1);

	// Send command to erase everything
	SPIBUFINIT;
	MachXO2_Command = MACHXO2_CMD_ERASE;
	pMachXO2_Operand[0] = 0x0F;
	MachXO2_SPITrans(4);

	// Look at busy status every so often until it is clear or until too much time goes by
	// The busy bit is in the MSB, but still means we can test nonzero
	do
	{
		// Do a wait between polls of the busy bit
		delay(100);
		timeout += 100;
		if(timeout > 5000) {
			delay(100);
			return -1;
		}

		// Go read the busy bit
		SPIBUFINIT;
		MachXO2_Command = MACHXO2_CMD_CHECK_BUSY;
		MachXO2_SPITrans(5);
		busy = pMachXO2_Data[0];
	} while(busy);

	delay(100);

	// Return success
	return 0;
}