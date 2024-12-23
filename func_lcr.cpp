#include "func_lcr.h"
#include <Arduino.h>
#include "board.h"
#include "settings.h"
#include "audio_design.h"
#include "MathHelpers.h"
#include "cmplx_helper.h"
#include "globals.h"
#include "displayhelp.h"
#include "src/utils/btn_bar_menu.h"
#include "apps.h"
#include <ili9341_t3n_font_Arial.h>
#include "sysmenu.h"
#include "helper.h"


static const uint AMPLITUDE_PRESETS_NUM = 3;

#define LCR_FUNC_CS       0
#define LCR_FUNC_CP       1
#define LCR_FUNC_LS       2
#define LCR_FUNC_LP       3
#define LCR_FUNC_RS       4
#define LCR_FUNC_RP       5
#define LCR_FUNC_Z        6
#define LCR_FUNC_Y        7
#define LCR_FUNC_G        8
#define LCR_FUNC_B        9
#define LCR_FUNC_X        10
#define LCR_FUNC_PHID     11
#define LCR_FUNC_PHIR     12
#define LCR_FUNC_D        13
#define LCR_FUNC_Q        14
#define LCR_FUNC_VAC      15
#define LCR_FUNC_IAC      16

#define LCR_FUNC_CS_RS        (LCR_FUNC_CS << 8) | LCR_FUNC_RS
#define LCR_FUNC_CS_D         (LCR_FUNC_CS << 8) | LCR_FUNC_D
#define LCR_FUNC_CP_RP        (LCR_FUNC_CP << 8) | LCR_FUNC_RP
#define LCR_FUNC_CP_D         (LCR_FUNC_CP << 8) | LCR_FUNC_D
#define LCR_FUNC_LP_RP        (LCR_FUNC_LP << 8) | LCR_FUNC_RP
#define LCR_FUNC_LP_Q         (LCR_FUNC_LP << 8) | LCR_FUNC_Q
#define LCR_FUNC_LS_RS        (LCR_FUNC_LS << 8) | LCR_FUNC_RS
#define LCR_FUNC_LS_Q         (LCR_FUNC_LS << 8) | LCR_FUNC_Q
#define LCR_FUNC_RS_Q         (LCR_FUNC_RS << 8) | LCR_FUNC_Q
#define LCR_FUNC_RP_Q         (LCR_FUNC_RP << 8) | LCR_FUNC_Q
#define LCR_FUNC_R_X          (LCR_FUNC_RS << 8) | LCR_FUNC_X
#define LCR_FUNC_Z_PHID       (LCR_FUNC_Z << 8) | LCR_FUNC_PHID
#define LCR_FUNC_Z_D          (LCR_FUNC_Z << 8) | LCR_FUNC_D
#define LCR_FUNC_Z_Q          (LCR_FUNC_Z << 8) | LCR_FUNC_Q
#define LCR_FUNC_G_B          (LCR_FUNC_G << 8) | LCR_FUNC_B

static const uint LCR_FUNC_NUM = 15;
static const uint lcr_func_presets[LCR_FUNC_NUM] = {
  LCR_FUNC_CS_RS,
  LCR_FUNC_CS_D,
  LCR_FUNC_CP_RP,
  LCR_FUNC_CP_D,
  LCR_FUNC_LP_RP,
  LCR_FUNC_LP_Q,
  LCR_FUNC_LS_RS,
  LCR_FUNC_LS_Q,
  LCR_FUNC_RS_Q,
  LCR_FUNC_RP_Q,
  LCR_FUNC_R_X, 
  LCR_FUNC_Z_PHID,
  LCR_FUNC_Z_D,
  LCR_FUNC_Z_Q,
  LCR_FUNC_G_B
};
typedef struct lcr_settings_struct {
  float frequency;
  uint8_t amplitudePreset;
  uint8_t displMode;
  uint8_t dcBias;
  uint8_t range_mode;
  uint8_t function;
  uint16_t averaging;
} lcr_settings_t;

lcr_settings_t lcrSettings = {
  .frequency = 1000,
  .amplitudePreset = 2,
  .displMode = 0,
  .dcBias = 0,
  .range_mode = 0,
  .function = 0,
  .averaging = 32,
};

bool rangingActive = false;
bool forceRanging = false;

// Menu definition
BtnBarMenu lcrMenu(&tft);
int lcrmenuBtnFreq, lcrmenuBtnLevel, lcrmenuBtnDisplMode, lcrmenuBtnRangeMode, lcrmenuBtnFunc, lcrmenuBtnSetAvg;
BtnBarMenu lcrSetFreqMenu(&tft);
int lcrmenuBtnF100, lcrmenuBtnF1k, lcrmenuBtnF10k, lcrmenuBtnFman;

const char *functionLabels[LCR_FUNC_NUM] = {
  "Cs-Rs", "Cs-D", "Cp-Rp", "Cp-D",
  "Lp-Rp", "Lp-Q", "Ls-Rs", "Ls-Q",
  "Rs-Q",  "Rp-Q", "R-X",   "Z-Phi",
  "Z-D",   "Z-Q", "G-B"
};
const char *functionLabelSelection;

static const float amplitudePresets[AMPLITUDE_PRESETS_NUM] = {0.3, 0.6, 1.0};
const char *levelLabels[AMPLITUDE_PRESETS_NUM] = {"300 mV", "600 mV", "1 V"};

const char *displModeLabels[] = {"Normal", "Debug"};
const char *displModeLabelSelection;

const char *rangeModeLabels[] = {"Auto", "Hold"};
const char *rangeModeLabelSelection;

typedef struct lcr_params_struct {
  float phi;  // phase angle of impedance
  float z;    // impedance (phasor)
  float y;    // admittance (phasor)
  float q;    // quality factor
  float d;    // dissipation factor
  float rs;   // equivalent series resistance (ESR)
  float g;    // conductance
  float cs;   // series capacitance
  float ls;   // series inductance
  float xs;   // series reactance
  float b;    // susceptance
  float rp;   // parallel resistance
  float cp;   // parallel capacitance
  float lp;   // parallel inductance
} lcr_params_t;

void printCalData() {
  Serial.println("calOutA");
  Serial.println(sci(calOutA.transmissionFactor, 6));
  Serial.println(sci(calOutA.gainFactor, 6));
  Serial.println("calInA");
  Serial.println(sci(calInA.transmissionFactor, 6));
  Serial.println(sci(calInA.gainFactor[0], 6));
  Serial.println(sci(calInA.gainFactor[1], 6));
  Serial.println(sci(calInA.gainFactor[2], 6));
  Serial.println(sci(calInA.gainFactor[3], 6));
  Serial.println("calInB");
  Serial.println(sci(calInB.transmissionFactor[0], 6));
  Serial.println(sci(calInB.transmissionFactor[1], 6));
  Serial.println(sci(calInB.transmissionFactor[2], 6));
  Serial.println(sci(calInB.transmissionFactor[3], 6));
  Serial.println(sci(calInB.gainFactor[0], 6));
  Serial.println(sci(calInB.gainFactor[1], 6));
  Serial.println(sci(calInB.gainFactor[2], 6));
  Serial.println(sci(calInB.gainFactor[3], 6));
}

cmplx_versor_t lcrCorrection(cmplx_versor_t zm, cmplx_versor_t zo) {
  // Zm = Zo || Zx
  // Zx = Zo * Zm / (Zo - Zm)
  cmplx_versor_t tmp1 = cmplxMultiplyVersor(zo, zm);
  cmplx_versor_t tmp2 = cmplxSubstractVersor(zo, zm);
  return cmplxDevideVersor(tmp1, tmp2);
}

lcr_params_t lcrCalcParams(float z, float phi, float f) {
  lcr_params_t results;

  // radian frequency in radians per second
  float omega = 2 * PI * f;

  results.z = z;
  results.phi = phi;
  results.y = 1 / results.z;
  results.rs = results.z * cos(results.phi);
  results.xs = results.z * sin(results.phi);
  results.g = 1 / results.rs;
  results.b = 1 / results.xs;
  results.q = abs(results.xs) / results.rs;
  results.d = 1 / results.q;
  results.rp = (1 + results.q * results.q) * results.rs;
  results.ls = abs(results.xs) / omega;
  results.lp = results.ls * (1 + results.d * results.d);
  results.cs = 1 / abs(results.xs) / omega;
  results.cp = results.cs / (1 + results.d * results.d);

  return results;
}

/*
 * Calc headroom expansion to compensate for codec attenuation > 40kHz
 */
float lcrHeadroomExp(float f)
{
  float hdr = 0;
  if (f > 40000.0)
  {
    hdr = (f - 40000.0) * 1.6e-4;
  }
  return hdr;
}

bool lcrAutoRange(float z)
{
  static const float LCR_MINIMUM_HEADROOM_V = 1.0; // dB
  static const float LCR_MINIMUM_HEADROOM_I = 1.0; // dB
  static const float LCR_HEADROOM_RANGE_V = 16.0;  // dB
  static const float LCR_HEADROOM_RANGE_I = 16.0;  // dB

  bool active = false;
  float v_headroom = adHeadroom(lcrReadings.v_peak);
  float i_headroom = adHeadroom(lcrReadings.i_peak);
  float hdrExp = lcrHeadroomExp(lcrSettings.frequency);
  
  // check headroom
  if (v_headroom < (LCR_MINIMUM_HEADROOM_V + hdrExp) && boardSettings.gain_v > 0)
  {
    // reduce voltage gain
    boardSetPGAGainV(--boardSettings.gain_v);
    active = true;
  }
  else if (v_headroom > (LCR_HEADROOM_RANGE_V + hdrExp) && boardSettings.gain_v < 3)
  {
    // increase voltage gain
    boardSetPGAGainV(++boardSettings.gain_v);
    active = true;
  }
  if (i_headroom < (LCR_MINIMUM_HEADROOM_I + hdrExp) && boardSettings.gain_i > 0)
  {
    // reduce current gain
    boardSetPGAGainI(--boardSettings.gain_i);
    active = true;
  }
  else if (i_headroom > (LCR_HEADROOM_RANGE_I + hdrExp) && boardSettings.gain_i < 3)
  {
    // increase current gain
    boardSetPGAGainI(++boardSettings.gain_i);
    active = true;
  }

  if (active)
    return true;
  
  if (lcrSettings.range_mode)
    return false;
  
  // check range with hysteresis
  if (z < 450)
  {
    if (boardSettings.range != 0)
    {
      boardSetLCRRange(0);
      return true;
    }
    else
      return false;
  }
  else if (z < 550)
  {
    if (boardSettings.range <= 1)
      return false;
    else
    {
      boardSetLCRRange(0);
      return true;
    }
  }
  else if (z < 4500)
  {
    if (boardSettings.range != 1)
    {
      boardSetLCRRange(1);
      return true;
    }
    else
      return false;
  }
  else if (z < 5500)
  {
    if(boardSettings.range == 1 || boardSettings.range == 2)
      return false;
    else
    {
      boardSetLCRRange(1);
      return true;
    }
  }
  else if (z < 45000)
  {
    if (boardSettings.range != 2)
    {
      boardSetLCRRange(2);
      return true;
    }
    else
      return false;
  }
  else if (z < 55000)
  {
    if (boardSettings.range >= 2)
      return false;
    else
    {
      boardSetLCRRange(2);
      return true;
    }
  }
  else
    if (lcrSettings.frequency > 11000)
    {
      if (boardSettings.range != 2)
      {
        boardSetLCRRange(2);
        return true;
      }
    }
    else
    {
      if (boardSettings.range != 3)
      {
        boardSetLCRRange(3);
        return true;
      }
    }
  return false;
}

void lcrDrawFuncIndicators()
{
  // skip if debug display
  if (lcrSettings.displMode != 0)
    return;

  uint preset = lcr_func_presets[lcrSettings.function];
  uint8_t primary_func = preset >> 8;
  uint8_t secondary_func = preset & 0xff;

  tft.fillRect(0, MAIN_DISPLAY_Y_PRIMARY, 31, 14, ILI9341_BLACK);
  tft.setFont(Arial_14);
  tft.setTextColor(ILI9341_GREEN);
  tft.setCursor(0, MAIN_DISPLAY_Y_PRIMARY);

  switch (primary_func)
  {
    case LCR_FUNC_CS:
      tft.print("Cs");
      break;
    case LCR_FUNC_CP:
      tft.print("Cp");
      break;
    case LCR_FUNC_RS:
      tft.print("Rs");
      break;
    case LCR_FUNC_RP:
      tft.print("Rp");
      break;
    case LCR_FUNC_LS:
      tft.print("Ls");
      break;
    case LCR_FUNC_LP:
      tft.print("Lp");
      break;
    case LCR_FUNC_Z:
      tft.print("Z");
      break;
    case LCR_FUNC_G:
      tft.print("G");
      break;
  }

  tft.fillRect(0, MAIN_DISPLAY_Y_SECONDARY, 31, 14, ILI9341_BLACK);
  tft.setCursor(0, MAIN_DISPLAY_Y_SECONDARY);

  switch (secondary_func)
  {
    case LCR_FUNC_RS:
      tft.print("Rs");
      break;
    case LCR_FUNC_RP:
      tft.print("Rp");
      break;
    case LCR_FUNC_D:
      tft.print("D");
      break;
    case LCR_FUNC_Q:
      tft.print("Q");
      break;
    case LCR_FUNC_X:
      tft.print("X");
      break;
    case LCR_FUNC_PHID:
      tft.print("Phi");
      break;
    case LCR_FUNC_PHIR:
      tft.print("Phi");
      break;
    case LCR_FUNC_Y:
      tft.print("Y");
      break;
    case LCR_FUNC_B:
      tft.print("B");
      break;
  }
}

void lcrDrawMeasSetup()
{
  // skip if debug display
  if (lcrSettings.displMode != 0)
    return;

  static const uint MEAS_SETUP_LINE1_Y = 123;
  static const uint MEAS_SETUP_LINE2_Y = 145;

  // clear area
  tft.fillRect(0, MEAS_SETUP_LINE1_Y, 320, 40, ILI9341_BLACK);
  // draw labels
  tft.setFont(Arial_14);
  tft.setTextColor(ILI9341_GREEN);
  tft.setCursor(6, MEAS_SETUP_LINE1_Y);
  tft.print("FREQ:");
  tft.setCursor(6, MEAS_SETUP_LINE2_Y);
  tft.print("LEVEL:");
  tft.setCursor(161, MEAS_SETUP_LINE1_Y);
  tft.print("RANGE:");
  //tft.print("FUNC:");
  tft.setCursor(161, MEAS_SETUP_LINE2_Y);
  tft.print("AVG:");

  // draw values
  tft.setTextColor(ILI9341_WHITE);

  tft.setCursor(80, MEAS_SETUP_LINE1_Y);
  if (lcrSettings.frequency < 1000) {
    tft.print(lcrSettings.frequency, 0);
    tft.print(" Hz");
  } else {
    tft.print(lcrSettings.frequency / 1000, 1);
    tft.print(" kHz");
  }

  tft.setCursor(80, MEAS_SETUP_LINE2_Y);
  tft.print(levelLabels[lcrSettings.amplitudePreset]);

  tft.setCursor(244, MEAS_SETUP_LINE1_Y);
  //tft.print(functionLabels[lcrSettings.function]);
  tft.print("[");
  tft.print(boardSettings.range);
  tft.print("] ");
  tft.print(rangeModeLabels[lcrSettings.range_mode]);

  tft.setCursor(244, MEAS_SETUP_LINE2_Y);
  tft.print(lcrSettings.averaging);
}

void calc_lcr() {
  static const float LCR_THRESHOLD_OPEN = 100e6; // Ohm

  float impedance = lcrReadings.v_rms / lcrReadings.i_rms;
  float phase = lcrReadings.phase;
  disp_val_t val;
  
  // determain lcr meter range
  bool ranging = lcrAutoRange(impedance);

  if (forceRanging || (!rangingActive && ranging))
  {
    // started ranging
    rangingActive = true;
    forceRanging = false;
    averaging = 1;
    adResetReadings();
    drawPrimaryDisplay(" -----");
    drawSecondaryDisplay(" -----");
    return;
  }
  if (rangingActive && ranging)
  {
    // ranging is active, readings are not valid yet.
    adResetReadings();
    return;
  }
  if (rangingActive && !ranging)
  {
    // finished ranging
    rangingActive = false;
    averaging = lcrSettings.averaging;
    lcrDrawMeasSetup();
    return;
  }

  lcr_params_t params = lcrCalcParams(impedance, phase, lcrSettings.frequency);

  uint preset = lcr_func_presets[lcrSettings.function];
  uint8_t primary_func = preset >> 8;
  uint8_t secondary_func = preset & 0xff;
  if (impedance > LCR_THRESHOLD_OPEN)
    primary_func = 0xff;
  
  switch (primary_func)
  {
    case LCR_FUNC_CS:
      getDisplValue(params.cs, 5, -13, &val);
      val.unit = "F";
      break;
    case LCR_FUNC_CP:
      getDisplValue(params.cp, 5, -13, &val);
      val.unit = "F";
      break;
    case LCR_FUNC_RS:
      getDisplValue(params.rs, 5, -4, &val);
      val.unit = "Ohm";
      break;
    case LCR_FUNC_RP:
      getDisplValue(params.rp, 5, -4, &val);
      val.unit = "Ohm";
      break;
    case LCR_FUNC_LS:
      getDisplValue(params.ls, 5, -8, &val);
      val.unit = "H";
      break;
    case LCR_FUNC_LP:
      getDisplValue(params.lp, 5, -8, &val);
      val.unit = "H";
      break;
    case LCR_FUNC_Z:
      getDisplValue(params.z, 5, -4, &val);
      val.unit = "Ohm";
      break;
    case LCR_FUNC_G:
      getDisplValue(params.g, 5, -8, &val);
      val.unit = "S";
      break;
    default:
      drawPrimaryDisplay(" Open");
      drawSecondaryDisplay("");
      return;
  }

  drawPrimaryDisplay(&val);
  
  switch (secondary_func)
  {
    case LCR_FUNC_RS:
      getDisplValue(params.rs, 5, -4, &val);
      val.unit = "Ohm";//String(252); // Ohm
      break;
    case LCR_FUNC_RP:
      getDisplValue(params.rp, 5, -4, &val);
      val.unit = "Ohm";//String(252); // Ohm
      break;
    case LCR_FUNC_D:
      getDisplValueExt(params.d, 5, -4, &val, true);
      val.unit = "";//String(251); // Theta
      break;
    case LCR_FUNC_Q:
      getDisplValueExt(params.q, 5, -4, &val, true);
      val.unit = "";//String(250); // Theta italic
      break;
    case LCR_FUNC_X:
      getDisplValue(params.xs, 5, -4, &val);
      val.unit = "Ohm";//String(252); // Ohm
      break;
    case LCR_FUNC_PHID:
      getDisplValueExt(params.phi / PI * 180, 5, -3, &val, true);
      val.unit = "o";//String(253); // deg
      break;
    case LCR_FUNC_PHIR:
      getDisplValueExt(params.phi, 5, -3, &val, true);
      val.unit = "rad";//String(253); // deg "o";
      break;
    case LCR_FUNC_Y:
      getDisplValue(params.y, 5, -4, &val);
      val.unit = "";
      break;
    case LCR_FUNC_B:
      getDisplValue(params.b, 5, -8, &val);
      val.unit = "S";
      break;
  }

  drawSecondaryDisplay(&val);
}

void calc_lcr2() {
  float impedance = lcrReadings.v_rms / lcrReadings.i_rms;
  float phase = lcrReadings.phase;
  disp_val_t val;
  
  tft.fillRect(0, 0, 320, tft.height() - 69, ILI9341_BLACK);
  tft.setFont();
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_GREEN);
  tft.setCursor(0, 0);
  tft.println("[ DEBUG DISPLAY ]");

  // determain lcr meter range
  bool ranging = false;
  if (lcrSettings.range_mode == 0)
    ranging = lcrAutoRange(impedance);

  if (forceRanging || (!rangingActive && ranging))
  {
    // started ranging
    rangingActive = true;
    forceRanging = false;
    averaging = 1;
    adResetReadings();
  }
  if (rangingActive && ranging)
  {
    // ranging is active, readings are not valid yet.
    adResetReadings();
  }
  if (rangingActive && !ranging)
  {
    // finished ranging
    rangingActive = false;
    averaging = lcrSettings.averaging;
  }

  lcr_params_t params = lcrCalcParams(impedance, phase, lcrSettings.frequency);

  tft.print("h V= ");
  tft.print(adHeadroom(lcrReadings.v_peak));
  tft.print("dB I= ");
  tft.print(adHeadroom(lcrReadings.i_peak));
  tft.println("dB");
  
  tft.print("V rms= ");
  tft.println(lcrReadings.v_rms, 6);
  //tft.print("I rms= ");
  //tft.println(sci(lcrReadings.i_rms, 6));
  tft.print("V mean= ");
  tft.println(sci(lcrReadings.v_mean, 6));
  tft.print("I mean= ");
  tft.println(sci(lcrReadings.i_mean, 6));
  
  tft.print("Z= ");
  tft.println(sci(impedance, 6));
  tft.print("Phi= ");
  tft.print(phase / PI * 180, 3);
  tft.print(" ");
  tft.println(lcrReadings.phase_raw / PI * 180, 3);

  //tft.print(sci(lcrReadings.mean2, 3));
  //tft.print(" ");
  //tft.println(sci(lcrReadings.mean4, 3));
  //tft.print(sci(lcrReadings.mean1, 3));
  //tft.print(" ");
  //tft.println(sci(lcrReadings.mean3, 3));
  
  //tft.print(lcrReadings.a1 / PI * 180, 3);
  //tft.print("   ");
  //tft.println(lcrReadings.a2 / PI * 180, 3);
  
  tft.print("Rs= ");
  tft.println(sci(params.rs, 6));
  /*
  
  Serial.println("correction");
  
  cmplx_versor_t zo;
  zo.z = 43915.0;
  zo.phi = 26.19;
  cmplx_versor_t zm;
  zm.z = params.z;
  zm.phi = params.phi;
  cmplx_versor_t z_cor = lcrCorrection(zm, zo);
  params = lcrCalcParams(z_cor.z, z_cor.phi, frequency_presets[frequency]);
  print_lcr_params(params);
  */
  //Serial.println();
  tft.print("Gv:");
  tft.print(boardSettings.gain_v);
  tft.print(" Gi:");
  tft.print(boardSettings.gain_i);
  tft.print(" R:");
  tft.print(boardSettings.range);

  if (rangingActive)
    tft.println(" ranging");
}

void lcrDrawMenu()
{
  if (activeMenu == APP_DEFAULT)
    lcrMenu.draw();
  else if (activeMenu == SELECT_FUNCTION)
    appSelectMenu.draw();
  else if (activeMenu == LCR_SET_FREQUENCY)
    lcrSetFreqMenu.draw();
  tft.updateScreen();
}

void lcrSetAveraging()
{
  tft.fillScreen(ILI9341_BLACK);
  tft.setFont(Arial_14);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(0, 0);
  tft.println("Averaging:");
  uint avg;
  bool ok = enterNr(&avg, 1, 256);
  if (ok)
  {
    lcrSettings.averaging = avg;
    averaging = avg;
    adResetReadings();
  }
  tft.fillScreen(ILI9341_BLACK);
  lcrDrawFuncIndicators();
  lcrDrawMeasSetup();
  lcrDrawMenu();
}

void lcr_select_func()
{
  lcrSettings.function = get_list_entry(functionLabels, LCR_FUNC_NUM, lcrSettings.function);
  functionLabelSelection = functionLabels[lcrSettings.function];
  tft.fillScreen(ILI9341_BLACK);
  lcrDrawMeasSetup();
  lcrDrawFuncIndicators();
  lcrDrawMenu();
}

void lcrSetFrequency(float f)
{
  if (f == 0) {
    tft.fillScreen(ILI9341_BLACK);
    tft.setFont(Arial_14);
    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(0, 0);
    tft.println("Enter frequency in kHz:");
    enterFrequency(&f, 90000.0);
  }
  if (f > 0) {
    lcrSettings.frequency = f;
    adSetOutputFrequency(lcrSettings.frequency);
    adResetSquarewavePhase();
    forceRanging = true;
    averaging = 1;
  }
  activeMenu = APP_DEFAULT;
  tft.fillScreen(ILI9341_BLACK);
  lcrDrawMeasSetup();
  lcrDrawFuncIndicators();
  lcrDrawMenu();
}

void lcrSetAmplitude()
{
  lcrSettings.amplitudePreset = ++lcrSettings.amplitudePreset % AMPLITUDE_PRESETS_NUM;
  adSetOutputAmplitude(amplitudePresets[lcrSettings.amplitudePreset] * sqrtf(2));
  forceRanging = true;
  averaging = 1;
  lcrDrawMeasSetup();
  lcrDrawMenu();
}

void lcrSetDisplayMode()
{
  lcrSettings.displMode = ++lcrSettings.displMode % 2;
  displModeLabelSelection = displModeLabels[lcrSettings.displMode];
  if (lcrSettings.displMode == 0) {
    tft.fillRect(0, 0, 320, tft.height() - 60, ILI9341_BLACK);
    lcrDrawFuncIndicators();
    lcrDrawMeasSetup();
  }
  lcrDrawMenu();
}

void lcrSetRangeMode()
{
  lcrSettings.range_mode = ++lcrSettings.range_mode % 2;
  rangeModeLabelSelection = rangeModeLabels[lcrSettings.range_mode];
  lcrDrawMeasSetup();
  lcrDrawMenu();
}

void lcrHandleButtons() {
  char key = keypad.getKey();
  if (!key)
    return;
  switch (key) {
    case '1':
      activeMenu = APP_DEFAULT;
      lcrDrawMenu();
      break;
    case '2':
      activeMenu = SELECT_FUNCTION;
      lcrDrawMenu();
      break;
    case '3':
      sysMenuShow();
      tft.fillScreen(ILI9341_BLACK);
      lcrDrawFuncIndicators();
      lcrDrawMeasSetup();
      lcrDrawMenu();
      break;
    case 'S':
      saveScreenshot(&tft);
      break;
    case 'L':
      boardSetPGAGainV(++boardSettings.gain_v % PGA_GAIN_NUM);
      adResetReadings();
      break;
    case 'R':
      boardSetPGAGainI(++boardSettings.gain_i % PGA_GAIN_NUM);
      adResetReadings();
      break;
    case 'C':
      boardSetLCRRange(++boardSettings.range % LCR_RANGE_NUM);
      adResetReadings();
      break;
    case '0':
      printCalData();
      break;
    default:
      break;
  }
}

bool lcrHandleTouch()
{
  TS_Point p;
  char key;
  if (!getTouchPoint(&p))
    return false;
    
  if (activeMenu == APP_DEFAULT) {
    key = lcrMenu.processTSPoint(p);
    if (key == BTN_BAR_MENU_EVENT_IDLE)
      return false;
    if (key == lcrmenuBtnFreq) {
      activeMenu = LCR_SET_FREQUENCY;
      lcrDrawMenu();
      return false;
    }
    else if (key == lcrmenuBtnLevel)
      lcrSetAmplitude();
    else if (key == lcrmenuBtnFunc)
      lcr_select_func();
    else if (key == lcrmenuBtnDisplMode)
      lcrSetDisplayMode();
    else if (key == lcrmenuBtnRangeMode)
      lcrSetRangeMode();
    else if (key == lcrmenuBtnSetAvg)
      lcrSetAveraging();
  }
  else if (activeMenu == SELECT_FUNCTION) {
    key = appSelectMenu.processTSPoint(p);
    if (key != BTN_BAR_MENU_EVENT_IDLE) {
      appId = key;
      return true;
    }
  }
  else if (activeMenu == LCR_SET_FREQUENCY) {
    key = lcrSetFreqMenu.processTSPoint(p);
    if (key != BTN_BAR_MENU_EVENT_IDLE) {
      if (key == lcrmenuBtnF100)
        lcrSetFrequency(100.0);
      else if (key == lcrmenuBtnF1k)
        lcrSetFrequency(1000.0);
      else if (key == lcrmenuBtnF10k)
        lcrSetFrequency(10000.0);
      else if (key == lcrmenuBtnFman) {
        lcrSetFrequency(0);
      }
    }
  }
  return false;
}

void lcrApplySettings()
{
  averaging = lcrSettings.averaging;
  functionLabelSelection = functionLabels[lcrSettings.function];
  rangeModeLabelSelection = rangeModeLabels[lcrSettings.range_mode];
  displModeLabelSelection = displModeLabels[lcrSettings.displMode];

  adSetOutputAmplitude(amplitudePresets[lcrSettings.amplitudePreset] * sqrtf(2));
  adSetOutputOffset(0);

  adSetOutputFrequency(lcrSettings.frequency);
  adResetSquarewavePhase();
}

void lcrApplication()
{
  // setup
  static const uint DISPLAY_UPDATE_RATE_MIN = 200;  // ms
  static elapsedMillis displayUpdate = 0;
  lcrApplySettings();
  
  lcrMenu.init(btn_feedback, "LCR Menu");
  lcrmenuBtnFreq = lcrMenu.add("Freq.");
  lcrmenuBtnLevel = lcrMenu.add("Level");
  lcrmenuBtnFunc = lcrMenu.add("Function", &functionLabelSelection);
  lcrmenuBtnRangeMode = lcrMenu.add("Range", &rangeModeLabelSelection);
  lcrmenuBtnSetAvg = lcrMenu.add("Avg.");
  lcrmenuBtnDisplMode = lcrMenu.add("Display", &displModeLabelSelection);

  lcrSetFreqMenu.init(btn_feedback, "Set Frequency");
  lcrmenuBtnF100 = lcrSetFreqMenu.add("100Hz");
  lcrmenuBtnF1k = lcrSetFreqMenu.add("1kHz");
  lcrmenuBtnF10k = lcrSetFreqMenu.add("10kHz");
  lcrmenuBtnFman = lcrSetFreqMenu.add("Manuell");


  tft.fillScreen(ILI9341_BLACK);
  lcrDrawFuncIndicators();
  lcrDrawMeasSetup();
  lcrDrawMenu();
  
  // loop
  while (1)
  {
    lcrHandleButtons();
    if (lcrHandleTouch())
      return;
    average_readings();

    if (displayUpdate >= DISPLAY_UPDATE_RATE_MIN && lcrDataAvailable)
    {
      lcrDataAvailable = false;
      displayUpdate = 0;
      osdMessage.clean();
      if (lcrSettings.displMode == 0)
        calc_lcr();
      else
        calc_lcr2();

      osdMessage.show();
      tft.updateScreen();
    }
  }
}
