#ifndef CORRECTION_H_
#define CORRECTION_H_

#include <Arduino.h>
#include "CComplex.h" // https://github.com/RobTillaart/Complex

static const uint8_t CORR_FREQ_COUNT = 38;
static const float corr_frequencies[CORR_FREQ_COUNT] = {
  10, 20, 30, 40, 50, 60, 80,
  100, 120, 150, 200, 250, 300, 400, 500, 600, 800,
  1000, 1200, 1500, 2000, 2500, 3000, 4000, 5000, 6000, 8000,
  10000, 12000, 15000, 20000, 25000, 30000, 40000, 50000, 60000, 80000, 90000
};

typedef struct corr_data_struct {
  time_t ts_open;
  time_t ts_short;
  Complex z0[CORR_FREQ_COUNT];
  Complex zs[CORR_FREQ_COUNT];
  Complex zp[CORR_FREQ_COUNT];
} corr_data_t;

extern corr_data_t corr_data;
extern bool corr_apply;

void corrApply(float *impedance, float *phase, float frequency);
void correctionMenu();

#endif
