#ifndef LCR_PARAM_H_
#define LCR_PARAM_H_

#include <Arduino.h>
#include "CComplex.h"

typedef struct lcr_param_struct {
  const char *label;
  const char *unit;
  int8_t resolution;
  float (*value)(Complex z, float f);
} lcr_param_t;

extern lcr_param_t lcrParamCs;
extern lcr_param_t lcrParamCp;
extern lcr_param_t lcrParamLs;
extern lcr_param_t lcrParamLp;
extern lcr_param_t lcrParamRs;
extern lcr_param_t lcrParamRp;
extern lcr_param_t lcrParamZ;
extern lcr_param_t lcrParamY;
extern lcr_param_t lcrParamG;
extern lcr_param_t lcrParamB;
extern lcr_param_t lcrParamXs;
extern lcr_param_t lcrParamPhiD;
extern lcr_param_t lcrParamPhiR;
extern lcr_param_t lcrParamD;
extern lcr_param_t lcrParamQ;

#endif // LCR_PARAM_H_
