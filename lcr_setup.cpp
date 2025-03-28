#include "lcr_setup.h"

lcr_settings_t lcrSettings = {
  .frequency = 1000,
  .level = 1,
  .displMode = 0,
  .range_mode = 0,
  .function = 0,
  .minAveraging = 32,
  .applyCorrection = false,
};

lcr_param_t *lcrParams[LCR_FUNC_NUM][2] = {
  {&lcrParamCs, &lcrParamRs},
  {&lcrParamCs, &lcrParamD},
  {&lcrParamCp, &lcrParamRp},
  {&lcrParamCp, &lcrParamD},
  {&lcrParamLp, &lcrParamRp},
  {&lcrParamLp, &lcrParamQ},
  {&lcrParamLs, &lcrParamRs},
  {&lcrParamLs, &lcrParamQ},
  {&lcrParamRs, &lcrParamQ},
  {&lcrParamRp, &lcrParamQ},
  {&lcrParamRs, &lcrParamXs},
  {&lcrParamZ, &lcrParamPhiD},
  {&lcrParamZ, &lcrParamD},
  {&lcrParamZ, &lcrParamQ},
  {&lcrParamG, &lcrParamB},
};

const char *functionLabels[LCR_FUNC_NUM] = {
  "Cs-Rs", "Cs-D", "Cp-Rp", "Cp-D",
  "Lp-Rp", "Lp-Q", "Ls-Rs", "Ls-Q",
  "Rs-Q",  "Rp-Q", "R-X",   "Z-Phi",
  "Z-D",   "Z-Q", "G-B"
};
