#include "EEPROMext.h"
#include <EEPROM.h>

int Int_eeprom::writeByte(const uint16_t memoryAddress, const uint8_t value)
{
  EEPROM.write(memoryAddress, value);
  return 0;
}

int Int_eeprom::writeBlock(const uint16_t memoryAddress, const uint8_t * buffer, const uint16_t length)
{
  uint16_t addr = memoryAddress;
  while (addr < memoryAddress + length)
  {
    EEPROM.write(addr++, *buffer++);
  }
  return 0;
}

int Int_eeprom::setBlock(const uint16_t memoryAddress, const uint8_t value, const uint16_t length)
{
  uint16_t addr = memoryAddress;
  while (addr < memoryAddress + length)
  {
    EEPROM.write(addr++, value);
  }
  return 0;
}

uint8_t Int_eeprom::readByte(const uint16_t memoryAddress)
{
  return EEPROM.read(memoryAddress);
}

uint16_t Int_eeprom::readBlock(const uint16_t memoryAddress, uint8_t * buffer, const uint16_t length)
{
  uint16_t addr = memoryAddress;
  while (addr < memoryAddress + length)
  {
    *buffer++ = EEPROM.read(addr++);
  }
  return length;
}

int Int_eeprom::updateByte(const uint16_t memoryAddress, const uint8_t data)
{
  if (data == readByte(memoryAddress)) return 0;
  return writeByte(memoryAddress, data);
}
