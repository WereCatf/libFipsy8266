# libFipsy8266

This is a simple library for making use of the Fipsy FPGA-boards. For now it's only for the ESP8266, but it can be extended to work on other boards, too, if needed.

# Usage

The class-constructor, fipsy8266Class(), accepts two parameters, namely which GPIO-pin to use as SPI chip-select (defaults to GPIO15) and which SPI-class to use (this is mostly for use with other boards with multiple SPI-buses):
```
fipsy8266Class(uint8_t csPin = 15, SPIClass *useSPI = &SPI)
```

The other important functions are:
```
void begin() -- self-explanatory
uint32_t readID() -- Returns 0xffffffff on error, otherwise the ID of the device
int8_t erase() -- Returns -1 on error, 0 on success
```
