#ifndef CORRECTION_H_
#define CORRECTION_H_

#include <Arduino.h>
#include "CComplex.h" // https://github.com/RobTillaart/Complex

typedef struct corr_data_struct {
  Complex z0;
  Complex zs;
  Complex zp;
  time_t z0_time;
  time_t zs_time;
  float f;
  bool apply;
} corr_data_t;

extern corr_data_t corr_data;

void corrApply(float *impedance, float *phase);
void correctionMenu();

#endif
