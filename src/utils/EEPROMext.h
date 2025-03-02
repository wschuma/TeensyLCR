#pragma once

#include "Arduino.h"


class Int_eeprom
{
public:
  /**
    * Initializes the EEPROM
    */
  Int_eeprom(void) {};

  bool     begin(void) { return true; };
  bool     isConnected(void) { return true; };


  //  writes a byte to memoryAddress
  //  returns status, 0 = OK
  int      writeByte(const uint16_t memoryAddress, const uint8_t value);
  
  //  writes length bytes from buffer to EEPROM
  //  returns status, 0 = OK
  int      writeBlock(const uint16_t memoryAddress, const uint8_t * buffer, const uint16_t length);
  //  set length bytes in the EEPROM to the same value.
  //  returns status, 0 = OK
  int      setBlock(const uint16_t memoryAddress, const uint8_t value, const uint16_t length);

  //  returns the value stored in memoryAddress
  uint8_t  readByte(const uint16_t memoryAddress);
  
  //  reads length bytes into buffer
  //  returns bytes read.
  uint16_t readBlock(const uint16_t memoryAddress, uint8_t * buffer, const uint16_t length);


  void     setExtraWriteCycleTime(uint8_t ms) {};

  //  returns 0 == OK
  int      updateByte(const uint16_t memoryAddress, const uint8_t data);
};

