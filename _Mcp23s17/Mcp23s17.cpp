/*
  MCP23S17.cpp  Version 0.1
  Microchip MCP23S17 SPI I/O Expander Class for Arduino
  Created by Cort Buffington & Keith Neufeld
  March, 2011

  Features Implemented (by word and bit):
	I/O Direction
	Pull-up on/off
	Input inversion
	Output write
	Input read

  Interrupt features are not implemented in this version
  byte based (portA, portB) functions are not implemented in this version

  NOTE:  Addresses below are only valid when IOCON.BANK=0 (register addressing mode)
		 This means one of the control register values can change register addresses!
		 The default values is 0, so that's how we're using it.

		 All registers except ICON (0xA and 0xB) are paired as A/B for each 8-bit GPIO port.
		 Comments identify the port's name, and notes on how it is used.

		 *THIS CLASS ENABLES THE ADDRESS PINS MCP_ON ALL CHIPS MCP_ON THE BUS WHEN THE FIRST CHIP OBJECT IS INSTANTIATED!

  USAGE: All Read/Write functions except wordWrite are implemented in two different ways.
		 Individual pin values are set by referencing "pin #" and On/Off, Input/Output or High/Low where
		 portA represents pins 0-7 and portB 8-15. So to set the most significant bit of portB, set pin # 15.
		 To Read/Write the values for the entire chip at once, a word mode is supported buy passing a
		 single argument to the function as 0x(portB)(portA). I/O mode Output is represented by 0.
		 The wordWrite function was to be used internally, but was made public for advanced users to have
		 direct and more efficient control by writing a value to a specific register pair.
*/

/////////////////////////////////////////////////////////////////////
#include <SPI.h>                 // Arduino IDE SPI library - uses AVR hardware SPI features
#include "_MCP23S17.h"            // Header files for this class
#include "Arduino.h"
/////////////////////////////////////////////////////////////////////
bool MCP::_spiInited = false;

/////////////////////////////////////////////////////////////////////
// Defines to keep logical information symbolic go here

/////////////////////////////////////////////////////////////////////
#define    MCP_HIGH          (1)
#define    MCP_LOW           (0)
#define    MCP_ON            (1)
#define    MCP_OFF           (0)
#define    MCP_OUTPUT        (0)
#define    MCP_INPUT         (1)

// Here we have things for the SPI bus configuration

#define    MCP_CLOCK_DIVIDER (8)           // SPI bus speed to be 1/2 of the processor clock speed - 8MHz on most Arduinos
//#define    SS            (53)          // SPI bus slave select output to pin 10 - READ ARDUINO SPI DOCS BEFORE CHANGING!!!

// Control byte and configuration register information - Control Byte: "0100 A2 A1 A0 R/W" -- W=0

#define    MCP_OPCODEW       (0b01000000)  // Opcode for MCP23S17 with LSB (bit0) set to write (0), address OR'd in later, bits 1-3
#define    MCP_OPCODER       (0b01000001)  // Opcode for MCP23S17 with LSB (bit0) set to read (1), address OR'd in later, bits 1-3

// IOCON configuration
#define	   MCP_MIRROR		 (0b01000000)  // Configuration register for MCP23S17, INT A and INT B Pins Mirror bit. (0 - disabled, 1 - enabled)
#define	   MCP_SEQO			 (0b00100000)  // Configuration register for MCP23S17, Sequential Operation mode bit. (1 - disabled, 0 - enabled)
#define	   MCP_DISSLW		 (0b00010000)  // Configuration register for MCP23S17, Slew Rate control bit for SDA output. (1 - disabled, 0 - enabled)
#define    MCP_HAEN			 (0b00001000)  // Configuration register for MCP23S17, the only thing we change is enabling hardware addressing
#define    MCP_ODR			 (0b00000100)  // Configuration register for MCP23S17, open-drain output (input-pull-up)
#define	   MCP_INTPO		 (0b00000010)  // Configuration register for MCP23S17, This bit sets the polarity of the INT output pin

/////////////////////////////////////////////////////////////////////
MCP::MCP()
{
	SetNotValid();
}

// Constructor to instantiate an instance of MCP to a specific chip (address)
/////////////////////////////////////////////////////////////////////
MCP::MCP(uint8_t address, uint8_t ssPin)
{
	Init(address, ssPin);
};

/////////////////////////////////////////////////////////////////////
void MCP::Init(uint8_t address, uint8_t ssPin)
{
	this->_address = address;
	this->_modeCache = 0xFFFF;                // Default I/O mode is all input, 0xFFFF
	this->_outputCache = 0x0000;                // Default output state is all off, 0x0000
	this->_pullupCache = 0x0000;                // Default pull-up state is all off, 0x0000
	this->_invertCache = 0x0000;                // Default input inversion state is not inverted, 0x0000
	this->_ssPin = ssPin;

	::pinMode(_ssPin, OUTPUT);
	//::digitalWrite(_ssPin, MCP_HIGH);

	if (false == MCP::_spiInited)
	{
		MCP::_spiInited = true;
		SPI.begin();                          // Start up the SPI bus crank'er up Charlie!
		SPI.beginTransaction(SPISettings(200000, MSBFIRST, SPI_MODE0));

		//SPI.setClockDivider(MCP_CLOCK_DIVIDER);   // Sets the SPI bus speed
		//SPI.setBitOrder(MSBFIRST);            // Sets SPI bus bit order (this is the default, setting it for good form!)
		//SPI.setDataMode(SPI_MODE0);           // Sets the SPI bus timing mode (this is the default, setting it for good form!)
	}
	this->byteWrite(IOCON, MCP_HAEN | MCP_ODR);
	SetValid();
}

/////////////////////////////////////////////////////////////////////
bool MCP::IsValid() const
{
	return m_isValid;
}

/////////////////////////////////////////////////////////////////////
void MCP::SetValid()
{
	m_isValid = true;
}

/////////////////////////////////////////////////////////////////////
void MCP::SetNotValid()
{
	m_isValid = false;
}

// GENERIC BYTE WRITE - will write a byte to a register, arguments are register address and the value to write
/////////////////////////////////////////////////////////////////////
void MCP::byteWrite(uint8_t reg, uint8_t value) 
{      // Accept the register and byte
  //PORTB &= 0b11111110;                                 // Direct port manipulation speeds taking Slave Select MCP_LOW before SPI action
	::digitalWrite(_ssPin, MCP_LOW);
	SPI.transfer(MCP_OPCODEW | (_address << 1));             // Send the MCP23S17 opcode, chip address, and write bit
	SPI.transfer(reg);                                   // Send the register we want to write
	SPI.transfer(value);                                 // Send the byte
	//PORTB |= 0b00000001;                                 // Direct port manipulation speeds taking Slave Select MCP_HIGH after SPI action
	::digitalWrite(_ssPin, MCP_HIGH);
}

// GENERIC WORD WRITE - will write a word to a register pair, LSB to first register, MSB to next higher value register 

/////////////////////////////////////////////////////////////////////
void MCP::wordWrite(uint8_t reg, unsigned int word)
{  // Accept the start register and word 
  //PORTB &= 0b11111011;                                 // Direct port manipulation speeds taking Slave Select MCP_LOW before SPI action 
	::digitalWrite(_ssPin, MCP_LOW);
	SPI.transfer(MCP_OPCODEW | (_address << 1));             // Send the MCP23S17 opcode, chip address, and write bit
	SPI.transfer(reg);                                   // Send the register we want to write 
	SPI.transfer((uint8_t)(word));                      // Send the low byte (register address pointer will auto-increment after write)
	SPI.transfer((uint8_t)(word >> 8));                 // Shift the high byte down to the low byte location and send
	//PORTB |= 0b00000100;                                 // Direct port manipulation speeds taking Slave Select MCP_HIGH after SPI action
	::digitalWrite(_ssPin, MCP_HIGH);
}

// MODE SETTING FUNCTIONS - BY PIN AND BY WORD
/////////////////////////////////////////////////////////////////////
void MCP::pinMode(uint8_t pin, uint8_t mode)
{  // Accept the pin # and I/O mode
	if (pin < 0 | pin > 15) return;               // If the pin value is not valid (1-16) return, do nothing and return
	if (mode == MCP_INPUT) {                          // Determine the mode before changing the bit state in the mode cache
		this->_modeCache |= 1 << (pin);               // Since input = "MCP_HIGH", OR in a 1 in the appropriate place
	}
	else {
		this->_modeCache &= ~(1 << (pin));            // If not, the mode must be output, so and in a 0 in the appropriate place
	}
	this->wordWrite(IODIRA, this->_modeCache);                // Call the generic word writer with start register and the mode cache
}

/////////////////////////////////////////////////////////////////////
void MCP::pinMode(unsigned int mode)
{    // Accept the word�
	this->wordWrite(IODIRA, mode);                // Call the the generic word writer with start register and the mode cache
	this->_modeCache = mode;
}

// THE FOLLOWING WRITE FUNCTIONS ARE NEARLY IDENTICAL TO THE FIRST AND ARE NOT INDIVIDUALLY COMMENTED

// WEAK PULL-UP SETTING FUNCTIONS - BY WORD AND BY PIN

/////////////////////////////////////////////////////////////////////
void MCP::pinMode_ALL_INPUT()
{
	this->pinMode(0xFFFF);     // Use word-write mode to set all of the pins on inputchip to be inputs
}

/////////////////////////////////////////////////////////////////////
void MCP::pinMode_ALL_INPUT_PULLUP()
{
	this->pinMode_ALL_INPUT();
	this->pullupMode(0xFFFF);  // Use word-write mode to Turn on the internal pull-up resistors.
}

/////////////////////////////////////////////////////////////////////
void MCP::pinMode_ALL_OUTPUT()
{
	this->pinMode(0x0000);    // Use word-write mode to Set all of the pins on outputchip to be outputs
}

/////////////////////////////////////////////////////////////////////
void MCP::pullupMode(uint8_t pin, uint8_t mode)
{
	if (pin < 0 | pin > 15) return;
	if (mode == MCP_ON) {
		this->_pullupCache |= 1 << (pin);
	}
	else {
		this->_pullupCache &= ~(1 << (pin));
	}
	this->wordWrite(GPPUA, this->_pullupCache);
}

/////////////////////////////////////////////////////////////////////
void MCP::pullupMode(unsigned int mode)
{
	this->wordWrite(GPPUA, mode);
	this->_pullupCache = mode;
}


// MCP_INPUT INVERSION SETTING FUNCTIONS - BY WORD AND BY PIN
/////////////////////////////////////////////////////////////////////
void MCP::inputInvert(uint8_t pin, uint8_t mode)
{
	if (pin < 0 | pin > 15) return;
	if (mode == MCP_ON) {
		this->_invertCache |= 1 << (pin);
	}
	else {
		this->_invertCache &= ~(1 << (pin));
	}
	this->wordWrite(IPOLA, this->_invertCache);
}

/////////////////////////////////////////////////////////////////////
void MCP::inputInvert(unsigned int mode)
{
	this->wordWrite(IPOLA, mode);
	this->_invertCache = mode;
}

/////////////////////////////////////////////////////////////////////
void MCP::interruptMode(unsigned int mode)
{
	this->wordWrite(DEFVALA, 0);
	this->wordWrite(GPINTENA, ~0);
	this->wordWrite(INTCONA, 0);
}

/////////////////////////////////////////////////////////////////////
unsigned int MCP::readInterruptSates()
{
	return (this->byteRead(INTFA)) | (this->byteRead(INTFB) << 8);
}

// WRITE FUNCTIONS - BY WORD AND BY PIN
/////////////////////////////////////////////////////////////////////
void MCP::digitalWrite(uint8_t pin, uint8_t value)
{
	if (pin < 0 | pin > 15) return;
	if (value) {
		this->_outputCache |= 1 << (pin);
	}
	else {
		this->_outputCache &= ~(1 << (pin));
	}
	this->wordWrite(GPIOA, this->_outputCache);
}

/////////////////////////////////////////////////////////////////////
void MCP::digitalWrite(unsigned int value)
{
	this->wordWrite(GPIOA, value);
	this->_outputCache = value;
}


// READ FUNCTIONS - BY WORD, BYTE AND BY PIN

/////////////////////////////////////////////////////////////////////
unsigned int MCP::digitalRead(void)
{       // This function will read all 16 bits of I/O, and return them as a word in the format 0x(portB)(portA)
 // unsigned int value = 0;                   // Initialize a variable to hold the read values to be returned
 // //PORTB &= 0b11111011;                      // Direct port manipulation speeds taking Slave Select MCP_LOW before SPI action
 // ::digitalWrite(_ssPin, MCP_LOW);
 // SPI.transfer(MCP_OPCODER | (_address << 1));  // Send the MCP23S17 opcode, chip address, and read bit
 // SPI.transfer(GPIOA);                      // Send the register we want to read
 // value = SPI.transfer(0x00);               // Send any byte, the function will return the read value (register address pointer will auto-increment after write)
 // value |= (SPI.transfer(0x00) << 8);       // Read in the "high byte" (portB) and shift it up to the high location and merge with the "low byte"
 // //PORTB |= 0b00000100;                      // Direct port manipulation speeds taking Slave Select MCP_HIGH after SPI action
 // ::digitalWrite(_ssPin, MCP_HIGH);
 // return value;                             // Return the constructed word, the format is 0x(portB)(portA)
	return this->wordRead(GPIOA);
}

/////////////////////////////////////////////////////////////////////
uint8_t MCP::byteRead(uint8_t reg)
{        // This function will read a single register, and return it
	uint8_t value = 0;                        // Initialize a variable to hold the read values to be returned
	//PORTB &= 0b11111011;                      // Direct port manipulation speeds taking Slave Select MCP_LOW before SPI action
	::digitalWrite(this->_ssPin, MCP_LOW);
	SPI.transfer(MCP_OPCODER | (this->_address << 1));  // Send the MCP23S17 opcode, chip address, and read bit
	SPI.transfer(reg);                        // Send the register we want to read
	value = SPI.transfer(0x00);               // Send any byte, the function will return the read value
	//PORTB |= 0b00000100;                      // Direct port manipulation speeds taking Slave Select MCP_HIGH after SPI action
	::digitalWrite(this->_ssPin, MCP_HIGH);
	return value;                             // Return the constructed word, the format is 0x(register value)
}

/////////////////////////////////////////////////////////////////////
uint16_t MCP::wordRead(uint8_t reg)
{        // This function will read a single register, and return it
	uint16_t value = 0;                       // Initialize a variable to hold the read values to be returned
											  //PORTB &= 0b11111011;                      // Direct port manipulation speeds taking Slave Select MCP_LOW before SPI action
	::digitalWrite(this->_ssPin, MCP_LOW);
	SPI.transfer(MCP_OPCODER | (this->_address << 1));  // Send the MCP23S17 opcode, chip address, and read bit
	SPI.transfer(reg);                        // Send the register we want to read
	value = SPI.transfer(0x00);               // Send any byte, the function will return the read value
	value |= (SPI.transfer(0x00) << 8);       // Read in the "high byte" (portB) and shift it up to the high location and merge with the "low byte"

											  //PORTB |= 0b00000100;                      // Direct port manipulation speeds taking Slave Select MCP_HIGH after SPI action
	::digitalWrite(this->_ssPin, MCP_HIGH);
	return value;                             // Return the constructed word, the format is 0x(register value)
}

/////////////////////////////////////////////////////////////////////
uint8_t MCP::digitalRead(uint8_t pin)
{              // Return a single bit value, supply the necessary bit (1-16)
	if (pin < 0 | pin > 15) return 0x0;                  // If the pin value is not valid (1-16) return, do nothing and return
	return this->digitalRead() & (1 << (pin)) ? MCP_HIGH : MCP_LOW;  // Call the word reading function, extract MCP_HIGH/MCP_LOW information from the requested pin
}