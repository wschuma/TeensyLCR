#ifndef LCR_SETUP_H_
#define LCR_SETUP_H_

#include <Arduino.h>
#include "lcr_param.h"

typedef struct lcr_settings_struct {
  float frequency;
  float level;
  uint8_t displMode;
  uint8_t range_mode;
  uint8_t function;
  uint8_t minAveraging;
  bool applyCorrection;
} lcr_settings_t;

extern lcr_settings_t lcrSettings;

static const uint LCR_FUNC_NUM = 15;
extern lcr_param_t *lcrParams[LCR_FUNC_NUM][2];
extern const char *functionLabels[LCR_FUNC_NUM];

#endif // LCR_SETUP_H_
