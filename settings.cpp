#include <Arduino.h>
#include "board.h"
#include "calibration.h"
#include "correction.h"
#include <EEPROM.h>
#include "globals.h"
#include "I2C_eeprom.h"
#include "settings.h"
#include "src/utils/EEPROMext.h"


#define SETTINGS_ID                   1773021235
#define SETTINGS_VERSION              0
#define EEPROM_SETTINGS_OFFSET        0
#define EEPROM_ID                     0   // 4 bytes
#define EEPROM_VERSION                4   // 1 byte
#define EEPROM_FUNCTION               10  // 1 byte
#define EEPROM_CALIBRATION            16  // calFactorOutput_t = 12 bytes, calFactorInputA_t = 24 bytes, calFactorInputB_t = 36 bytes; 72 bytes
#define EEPROM_CORRECTION             1024


uint appId;

#ifndef USE_INTERNAL_EEPROM
I2C_eeprom ee(I2C_ADDR_EEPROM, I2C_DEVICESIZE_24LC64);
#else
Int_eeprom ee;
#endif

void loadCalibrationData();
void loadCorrectionData();
void loadFunction();

/*
 * Initialize settings with default values.
 */
void initSettings()
{
#ifdef DBG_VERBOSE
  Serial.println("initSettings");
#endif

  // init correction data
  corr_data.ts_open = 0;
  corr_data.ts_short = 0;

  appId = 0;
}

/*
 * Initialize EEPROM data.
 */
void initEeprom()
{
#ifdef DBG_VERBOSE
  Serial.println("initEeprom");
#endif

  tft.println("Initialize EEPROM...");
  
  if (!ee.isConnected())
  {
    Serial.println("ERROR: Can't find EEPROM.");
    return;
  }

  delay(5);
  // write id
  uint32_t sid = SETTINGS_ID;
  ee.writeBlock(EEPROM_SETTINGS_OFFSET + EEPROM_ID, (uint8_t *) &sid, 4);
  ee.writeByte(EEPROM_SETTINGS_OFFSET + EEPROM_VERSION, SETTINGS_VERSION);

  // write settings
  saveCalibrationData();
  saveFunction();

  tft.println("done");
}

/*
 * Load settings from EEPROM.
 */
void loadSettings()
{
#ifdef DBG_VERBOSE
  Serial.println("loadSettings");
#endif

  ee.begin();
  ee.setExtraWriteCycleTime(10);

  if (!ee.isConnected())
  {
    Serial.println("ERROR: Can't find EEPROM.");
    return;
  }

  // EEPROM check
  uint32_t id;
  int status = ee.readBlock(EEPROM_SETTINGS_OFFSET + EEPROM_ID, (uint8_t *) &id, 4);
#ifdef DBG_VERBOSE
  Serial.println(status);
  Serial.println(id);
#endif
  if (id != SETTINGS_ID)
  {
    tft.println("EEPROM not initialized");
    Serial.println("EEPROM not initialized");
    initEeprom();
    return;
  }

  loadFunction();
  loadCalibrationData();
  loadCorrectionData();
}

int saveCalibrationData()
{
#ifdef DBG_VERBOSE
  Serial.println("saveCalibrationData");
#endif

  int status;
  int block = 0;
  uint addr = EEPROM_SETTINGS_OFFSET + EEPROM_CALIBRATION;
  
  if (!ee.isConnected())
  {
    Serial.println("ERROR: Can't find EEPROM.");
    return -1;
  }

  while (1)
  {
    delay(5);
    status = ee.setBlock(addr, 0, sizeof(calInA));
    if (status != 0)
      break;
    block = 1;
    status = ee.writeBlock(addr, (uint8_t *) &calInA, sizeof(calInA));
    if (status != 0)
      break;
    //ee.readBlock(addr, (uint8_t *) &calInA, sizeof(calInA));
    addr += sizeof(calInA);
    delay(5);
    block = 0;
    //status = ee.setBlock(addr, 0, sizeof(calInB));
    if (status != 0)
      break;
    block = 1;
    status = ee.writeBlock(addr, (uint8_t *) &calInB, sizeof(calInB));
    if (status != 0)
      break;
    addr += sizeof(calInB);
    delay(5);
    block = 0;
    //status = ee.setBlock(addr, 0, sizeof(calOutA));
    if (status != 0)
      break;
    block = 1;
    status = ee.writeBlock(addr, (uint8_t *) &calOutA, sizeof(calOutA));
    break;
  }
  if (status != 0)
  {
    Serial.println("Error saving data to EEPROM.");
    Serial.print("status = 0x");
    Serial.println(status, HEX);
    Serial.print("addr = 0x");
    Serial.println(addr, HEX);
    Serial.print("block = ");
    Serial.println(block);
  }
  return status;
}

int saveCorrectionData()
{
#ifdef DBG_VERBOSE
  Serial.println("saveCorrectionData");
#endif

  uint addr = EEPROM_SETTINGS_OFFSET + EEPROM_CORRECTION;
  int status;
  
  if (!ee.isConnected())
  {
    Serial.println("ERROR: Can't find EEPROM.");
    return -1;
  }

  delay(5);
  while (1)
  {
    // save time stamps
    status = ee.writeBlock(addr, (uint8_t *) &corr_data.ts_open, sizeof(int));
    if (status != 0)
      break;
    addr += sizeof(int);
    delay(5);
    status = ee.writeBlock(addr, (uint8_t *) &corr_data.ts_short, sizeof(int));
    if (status != 0)
      break;
    addr += sizeof(int);

    // save values
    float val[2];
    for (uint8_t point = 0; point < CORR_FREQ_COUNT; point++)
    {
      delay(5);
      val[0] = corr_data.z0[point].real();
      val[1] = corr_data.z0[point].imag();
      status = ee.writeBlock(addr, (uint8_t *) &val, sizeof(val));
      addr += sizeof(val);
      if (status != 0)
        break;
    }
    if (status != 0)
      break;
    for (uint8_t point = 0; point < CORR_FREQ_COUNT; point++)
    {
      delay(5);
      val[0] = corr_data.zs[point].real();
      val[1] = corr_data.zs[point].imag();
      status = ee.writeBlock(addr, (uint8_t *) &val, sizeof(val));
      addr += sizeof(val);
      if (status != 0)
        break;
    }
    if (status != 0)
      break;
    for (uint8_t point = 0; point < CORR_FREQ_COUNT; point++)
    {
      delay(5);
      val[0] = corr_data.zp[point].real();
      val[1] = corr_data.zp[point].imag();
      status = ee.writeBlock(addr, (uint8_t *) &val, sizeof(val));
      addr += sizeof(val);
      if (status != 0)
        break;
    }
    break;
  }

  if (status != 0)
  {
    Serial.println("Error saving data to EEPROM.");
    Serial.print("status = 0x");
    Serial.println(status, HEX);
    Serial.print("addr = 0x");
    Serial.println(addr, HEX);
  }
  return status;
}

void loadCalibrationData()
{
#ifdef DBG_VERBOSE
  Serial.println("loadCalibrationData");
#endif

  if (!ee.isConnected())
  {
    Serial.println("ERROR: Can't find EEPROM.");
    return;
  }

  uint addr = EEPROM_SETTINGS_OFFSET + EEPROM_CALIBRATION;
  ee.readBlock(addr, (uint8_t *) &calInA, sizeof(calInA));
  addr += sizeof(calInA);
  ee.readBlock(addr, (uint8_t *) &calInB, sizeof(calInB));
  addr += sizeof(calInB);
  ee.readBlock(addr, (uint8_t *) &calOutA, sizeof(calOutA));
}

void loadCorrectionData()
{
#ifdef DBG_VERBOSE
  Serial.println("loadCorrectionData");
#endif

  uint addr = EEPROM_SETTINGS_OFFSET + EEPROM_CORRECTION;
  // read time stamps
  uint16_t bytes = ee.readBlock(addr, (uint8_t *) &corr_data.ts_open, 4);
  addr += 4;
  bytes += ee.readBlock(addr, (uint8_t *) &corr_data.ts_short, 4);
  addr += 4;

  // reset timestamp if nothing is stored yet
  if (corr_data.ts_open == (uint16_t)-1)
    corr_data.ts_open = 0;
  if (corr_data.ts_short == (uint16_t)-1)
    corr_data.ts_short = 0;

  // read values
  float buffer[CORR_FREQ_COUNT][2];
  bytes += ee.readBlock(addr, (uint8_t *) &buffer, sizeof(buffer));
  addr += sizeof(buffer);
  for (uint8_t point = 0; point < CORR_FREQ_COUNT; point++)
  {
    corr_data.z0[point].set(buffer[point][0], buffer[point][1]);
  }
  bytes += ee.readBlock(addr, (uint8_t *) &buffer, sizeof(buffer));
  addr += sizeof(buffer);
  for (uint8_t point = 0; point < CORR_FREQ_COUNT; point++)
  {
    corr_data.zs[point].set(buffer[point][0], buffer[point][1]);
  }
  bytes += ee.readBlock(addr, (uint8_t *) &buffer, sizeof(buffer));
  for (uint8_t point = 0; point < CORR_FREQ_COUNT; point++)
  {
    corr_data.zp[point].set(buffer[point][0], buffer[point][1]);
  }


#ifdef DBG_VERBOSE
  Serial.print(bytes);
  Serial.println(" bytes loaded.");
#endif
}

void saveFunction()
{
#ifdef DBG_VERBOSE
  Serial.println("saveFunction");
  Serial.println(appId);
#endif
  
  ee.updateByte(EEPROM_SETTINGS_OFFSET + EEPROM_FUNCTION, appId);
}

void loadFunction()
{
#ifdef DBG_VERBOSE
  Serial.println("loadFunction");
#endif
  
  appId = ee.readByte(EEPROM_SETTINGS_OFFSET + EEPROM_FUNCTION);

#ifdef DBG_VERBOSE
  Serial.println(appId);
#endif
}

void copyExtToIntEeprom()
{
  const uint BLENGTH = 256;
  uint8_t data[BLENGTH];
  ee.readBlock(0, data, BLENGTH);
  for (uint addr = 0; addr < BLENGTH; addr++)
  {
    EEPROM.write(addr, data[addr]);
  }
}

void copyIntToExtEeprom()
{
  const uint BLENGTH = 96;
  uint8_t data[BLENGTH];
  for (uint addr = 0; addr < BLENGTH; addr++)
  {
    data[addr] = EEPROM.read(addr);
  }
  ee.writeBlock(0, data, BLENGTH);
}
