#ifndef calibration_h
#define calibration_h


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

extern calFactorOutput_t calOutA;
extern calFactorInputA_t calInA;
extern calFactorInputB_t calInB;

void calInit();
void functionCalib();

#endif