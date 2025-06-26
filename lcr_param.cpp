#include "lcr_param.h"


float getValueCs(Complex z, float f)
{
  float omega = 2 * PI * f;
  return 1 / abs(z.imag()) / omega;
}

float getValueCp(Complex z, float f)
{
  float omega = 2 * PI * f;
  float d = z.real() / abs(z.imag());
  return 1 / abs(z.imag()) / omega / (1 + d * d);
}

float getValueLs(Complex z, float f)
{
  float omega = 2 * PI * f;
  return abs(z.imag()) / omega;
}

float getValueLp(Complex z, float f)
{
  float omega = 2 * PI * f;
  float d = z.real() / abs(z.imag());
  return abs(z.imag()) / omega / (1 + d * d);
}

float getValuePhiR(Complex z, float f)
{
  return z.phase();
}

float getValuePhiD(Complex z, float f)
{
  return z.phase() * 180 / PI;
}

float getValueZ(Complex z, float f)
{
  return z.modulus();
}

float getValueY(Complex z, float f)
{
  return 1 / z.modulus();
}

float getValueRs(Complex z, float f)
{
  return z.real();
}

float getValueRp(Complex z, float f)
{
  float q = abs(z.imag()) / z.real();
  return z.real() * (1 + q * q);
}

float getValueXs(Complex z, float f)
{
  return z.imag();
}

float getValueG(Complex z, float f)
{
  return 1 / z.real();
}

float getValueB(Complex z, float f)
{
  return 1 / z.imag();
}

float getValueQ(Complex z, float f)
{
  return abs(z.imag()) / z.real();
}

float getValueD(Complex z, float f)
{
  return z.real() / abs(z.imag());
}

lcr_param_t lcrParamCs {
  .label = "Cs",
  .unit = "F",
  .resolution = -15,
  .value = &getValueCs,
};

lcr_param_t lcrParamCp {
  .label = "Cp",
  .unit = "F",
  .resolution = -15,
  .value = &getValueCp,
};

lcr_param_t lcrParamLs {
  .label = "Ls",
  .unit = "H",
  .resolution = -8,
  .value = &getValueLs,
};

lcr_param_t lcrParamLp {
  .label = "Lp",
  .unit = "H",
  .resolution = -8,
  .value = &getValueLp,
};

lcr_param_t lcrParamRs {
  .label = "Rs",
  .unit = "Ohm",
  .resolution = -5,
  .value = &getValueRs,
};

lcr_param_t lcrParamRp {
  .label = "Rp",
  .unit = "Ohm",
  .resolution = -5,
  .value = &getValueRp,
};

lcr_param_t lcrParamZ {
  .label = "Z",
  .unit = "Ohm",
  .resolution = -5,
  .value = &getValueZ,
};

lcr_param_t lcrParamY {
  .label = "Y",
  .unit = "",
  .resolution = -4,
  .value = &getValueY,
};

lcr_param_t lcrParamG {
  .label = "G",
  .unit = "S",
  .resolution = -8,
  .value = &getValueG,
};

lcr_param_t lcrParamB {
  .label = "B",
  .unit = "S",
  .resolution = -8,
  .value = &getValueB,
};

lcr_param_t lcrParamXs {
  .label = "X",
  .unit = "Ohm",
  .resolution = -5,
  .value = &getValueXs,
};

lcr_param_t lcrParamPhiD {
  .label = "Phi",
  .unit = "o",
  .resolution = -1,
  .value = &getValuePhiD,
};

lcr_param_t lcrParamPhiR {
  .label = "Phi",
  .unit = "rad",
  .resolution = -3,
  .value = &getValuePhiR,
};

lcr_param_t lcrParamD {
  .label = "D",
  .unit = "",
  .resolution = -4,
  .value = &getValueD,
};

lcr_param_t lcrParamQ {
  .label = "Q",
  .unit = "",
  .resolution = -4,
  .value = &getValueQ,
};
