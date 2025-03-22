#ifndef settings_h
#define settings_h

#include "Arduino.h"


extern uint appId;


void initSettings();
void loadSettings();
int saveCalibrationData();
int saveCorrectionData();
void saveFunction();
void copyExtToIntEeprom();
void copyIntToExtEeprom();

#endif