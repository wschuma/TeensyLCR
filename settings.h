#ifndef settings_h
#define settings_h

#include "Arduino.h"
#include "board.h"


typedef struct boardSettingsStruct {
  uint gain_v;
  uint gain_i;
  uint range;
} boardSettings_t;

extern uint appId;
extern boardSettings_t boardSettings;

extern const uint gain_v_presets[];
extern const uint gain_i_presets[];
extern const uint range_presets[];

void initSettings();
void loadSettings();
int saveCalibrationData();
int saveCorrectionData();
void saveFunction();
void copyExtToIntEeprom();
void copyIntToExtEeprom();

#endif