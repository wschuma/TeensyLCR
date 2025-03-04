#ifndef settings_h
#define settings_h

#include "Arduino.h"
#include "board.h"


#define TRANSMISSION_FACTOR_OUTPUT_A  0.4f    // 1/V
#define CALIBRATION_FACTOR_OUT_A      0.887f
#define CALIBRATION_FACTOR_OUT_B      0.8879f

#define TRANSMISSION_FACTOR_INPUT_A   2.82486f  // V/1
#define CALIBRATION_FACTOR_IN_A       1.00148f

#define TRANSMISSION_FACTOR_INPUT_B   2.82486e-3f  // A/1
#define CALIBRATION_FACTOR_IN_B       1.00943f




// calibration factors
typedef struct calFactorOutputStruct {
  float transmissionFactor;
  float gainFactor;
  float offset;
} calFactorOutput_t;

typedef struct calFactorInputAStruct {
  float transmissionFactor;
  float gainFactor[PGA_GAIN_NUM];
  float offset;
} calFactorInputA_t;

typedef struct calFactorInputBStruct {
  float transmissionFactor[LCR_RANGE_NUM];
  float gainFactor[PGA_GAIN_NUM];
  float offset;
} calFactorInputB_t;

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

extern calFactorOutput_t calOutA;
extern calFactorInputA_t calInA;
extern calFactorInputB_t calInB;


void initSettings();
void loadSettings();
int saveCalibrationData();
int saveCorrectionData();
void saveFunction();
void copyExtToIntEeprom();
void copyIntToExtEeprom();

#endif