#include <Arduino.h>
#include "settings.h"
#include "globals.h"
#include "I2C_eeprom.h"


#define SETTINGS_ID                   1773021235
#define SETTINGS_VERSION              0
#define EEPROM_SETTINGS_OFFSET        0
#define EEPROM_ID                     0   // 4 bytes
#define EEPROM_VERSION                4   // 1 byte
#define EEPROM_FUNCTION               10  // 1 byte
#define EEPROM_CALIBRATION            16

const uint gain_v_presets[] = {PGA_GAIN_1, PGA_GAIN_5, PGA_GAIN_25, PGA_GAIN_100};
const uint gain_i_presets[] = {PGA_GAIN_1, PGA_GAIN_5, PGA_GAIN_25, PGA_GAIN_100};
const uint range_presets[] = {LCR_RANGE_100, LCR_RANGE_1K, LCR_RANGE_10K, LCR_RANGE_100K};

calFactorOutput_t calOutA;
calFactorInputA_t calInA;
calFactorInputB_t calInB;

boardSettings_t boardSettings;
uint appId;

I2C_eeprom ee(I2C_ADDR_EEPROM, I2C_DEVICESIZE_24LC02);

void loadCalibrationData();
void loadFunction();

/*
 * Initialize settings with default values.
 */
void initSettings()
{
#ifdef DBG_VERBOSE
  Serial.println("initSettings");
#endif

  boardSettings.gain_v = 0;
  boardSettings.gain_i = 0;
  boardSettings.range = 0;

  // init calibration data
  calOutA.offset = 0;
  calOutA.transmissionFactor = 0.4; // 1/V
  calOutA.gainFactor = 1.087;
  
  calInA.offset = 0;
  calInA.transmissionFactor = 2.5; // V/1
  calInA.gainFactor[0] = 1.079;
  calInA.gainFactor[1] = 0.1923;
  calInA.gainFactor[2] = 3.984e-2;
  calInA.gainFactor[3] = 9.96e-3;
  
  calInB.offset = 0;
  calInB.transmissionFactor[0] = 2.5e-2; // A/1
  calInB.transmissionFactor[1] = 2.5e-3;
  calInB.transmissionFactor[2] = 2.5e-4;
  calInB.transmissionFactor[3] = 2.5e-5;
  calInB.gainFactor[0] = 1.0;
  calInB.gainFactor[1] = 0.1923;
  calInB.gainFactor[2] = 3.984e-2;
  calInB.gainFactor[3] = 9.96e-3;

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
  union ArrayToInteger {
    byte array[4];
    uint32_t integer;
  };
  ArrayToInteger id;
  ee.readBlock(EEPROM_SETTINGS_OFFSET + EEPROM_ID, (uint8_t *) &id.array, 4);
  if (id.integer != SETTINGS_ID)
  {
    tft.println("EEPROM not initialized");
    Serial.println("EEPROM not initialized");
    initEeprom();
    return;
  }

  loadFunction();
  loadCalibrationData();
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
