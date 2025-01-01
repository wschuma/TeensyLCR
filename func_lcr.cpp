#include "func_lcr.h"
#include <Arduino.h>
#include "autorange.h"
#include "board.h"
#include "correction.h"
#include "settings.h"
#include "audio_design.h"
#include "MathHelpers.h"
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

bool forceRanging = false;

// Menu definition
BtnBarMenu lcrMenu(&tft);
int lcrmenuBtnFreq, lcrmenuBtnLevel, lcrmenuBtnDisplMode, lcrmenuBtnRangeMode, lcrmenuBtnFunc, lcrmenuBtnSetAvg, lcrmenuBtnCorr;
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

const char *offOnLabels[] = {"OFF", "ON"};
const char *corrLabelSelection;
bool applyCorrection = false;

char avgStr[6];

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

void lcrUpdateCorrState()
{
  if (corr_data.f == lcrSettings.frequency && corr_data.apply &&
      (corr_data.z0_time > 0 || corr_data.zs_time > 0))
  {
    corrLabelSelection = offOnLabels[1];
    applyCorrection = true;
  } else {
    corrLabelSelection = offOnLabels[0];
    applyCorrection = false;
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
  tft.print("CORR:");

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
  tft.print(corrLabelSelection);
}

void calc_lcr() {
  static const float LCR_THRESHOLD_OPEN = 100e6; // Ohm

  // determain lcr meter range
  bool hold = lcrSettings.range_mode == 1;
  RangingState state = autoRange(hold, forceRanging);
  switch (state) {
    case (RangingState::Started):
      // ranging started
      forceRanging = false;
      drawPrimaryDisplay(" -----");
      drawSecondaryDisplay(" -----");
      return;
    case (RangingState::Active):
      // ranging is active, readings are not valid yet
      return;
    case (RangingState::Finished):
      // finished ranging, restore averaging
      adSetAveraging(lcrSettings.averaging);
      lcrDrawMeasSetup();
      return;
    default:
      // active range is ok, show results
      break;
  }

  float impedance = adReadings.v_rms / adReadings.i_rms;
  float phase = adReadings.phase;
  disp_val_t val;
  
  if (applyCorrection)
    corrApply(&impedance, &phase);

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
  tft.fillRect(0, 0, 320, tft.height() - 69, ILI9341_BLACK);
  tft.setFont();
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_GREEN);
  tft.setCursor(0, 0);
  tft.println("[ DEBUG DISPLAY ]");

  // determain lcr meter range
  bool hold = lcrSettings.range_mode == 1;
  RangingState state = autoRange(hold, forceRanging);
  switch (state) {
    case (RangingState::Started):
      // ranging started
      forceRanging = false;
      break;
    case (RangingState::Finished):
      // finished ranging, restore averaging
      adSetAveraging(lcrSettings.averaging);
      break;
    default:
      // active range is ok, show results
      break;
  }

  float impedance = adReadings.v_rms / adReadings.i_rms;
  float phase = adReadings.phase;
  disp_val_t val;
  
  if (applyCorrection)
    corrApply(&impedance, &phase);

  lcr_params_t params = lcrCalcParams(impedance, phase, lcrSettings.frequency);

  tft.print("h V= ");
  tft.print(adHeadroom(adReadings.v_peak));
  tft.print("dB I= ");
  tft.print(adHeadroom(adReadings.i_peak));
  tft.println("dB");
  
  tft.print("V rms= ");
  tft.println(adReadings.v_rms, 6);
  //tft.print("I rms= ");
  //tft.println(sci(adReadings.i_rms, 6));
  tft.print("V mean= ");
  tft.println(sci(adReadings.v_mean, 6));
  tft.print("I mean= ");
  tft.println(sci(adReadings.i_mean, 6));
  
  tft.print("Z= ");
  tft.println(sci(impedance, 6));
  tft.print("Phi= ");
  tft.print(phase / PI * 180, 3);
  tft.print(" ");
  tft.println(adReadings.phase_raw / PI * 180, 3);

  //tft.print(sci(adReadings.mean2, 3));
  //tft.print(" ");
  //tft.println(sci(adReadings.mean4, 3));
  //tft.print(sci(adReadings.mean1, 3));
  //tft.print(" ");
  //tft.println(sci(adReadings.mean3, 3));
  
  //tft.print(adReadings.a1 / PI * 180, 3);
  //tft.print("   ");
  //tft.println(adReadings.a2 / PI * 180, 3);
  
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

  if (state == RangingState::Active)
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
  
  osdMessage.show();
  tft.updateScreen();
  osdMessage.clean();
}

void lcrResetScreen()
{
  tft.fillScreen(ILI9341_BLACK);
  lcrDrawFuncIndicators();
  lcrDrawMeasSetup();
  lcrDrawMenu();
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
    adSetAveraging(avg);
    sprintf(avgStr, "%i", avg);
  }
  lcrResetScreen();
}

void lcr_select_func()
{
  lcrSettings.function = get_list_entry(functionLabels, LCR_FUNC_NUM, lcrSettings.function);
  functionLabelSelection = functionLabels[lcrSettings.function];
  lcrResetScreen();
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
    adSetAveraging(1);
    lcrUpdateCorrState();
  }
  activeMenu = APP_DEFAULT;
  lcrResetScreen();
}

void lcrSetAmplitude()
{
  lcrSettings.amplitudePreset = ++lcrSettings.amplitudePreset % AMPLITUDE_PRESETS_NUM;
  adSetOutputAmplitude(amplitudePresets[lcrSettings.amplitudePreset] * sqrtf(2));
  forceRanging = true;
  adSetAveraging(1);
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

void lcrApplySettings()
{
  adSetAveraging(lcrSettings.averaging);
  sprintf(avgStr, "%i", lcrSettings.averaging);
  functionLabelSelection = functionLabels[lcrSettings.function];
  rangeModeLabelSelection = rangeModeLabels[lcrSettings.range_mode];
  displModeLabelSelection = displModeLabels[lcrSettings.displMode];
  lcrUpdateCorrState();

  adSetOutputAmplitude(amplitudePresets[lcrSettings.amplitudePreset] * sqrtf(2));
  adSetOutputOffset(0);

  adSetOutputFrequency(lcrSettings.frequency);
  adResetSquarewavePhase();
}

void lcrCorrectionMenu()
{
  correctionMenu();
  lcrSettings.range_mode = 0;
  lcrApplySettings();
  lcrResetScreen();
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
      lcrResetScreen();
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
    else if (key == lcrmenuBtnCorr)
      lcrCorrectionMenu();
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

void lcrApplication()
{
  // setup
  static const uint DISPLAY_UPDATE_RATE_MIN = 200;  // ms
  static elapsedMillis displayUpdate = 0;
  const char *avgStrPtr = &avgStr[0];
  lcrApplySettings();
  
  lcrMenu.init(btn_feedback, "LCR Menu");
  lcrmenuBtnFreq = lcrMenu.add("Freq.");
  lcrmenuBtnLevel = lcrMenu.add("Level");
  lcrmenuBtnFunc = lcrMenu.add("Function", &functionLabelSelection);
  lcrmenuBtnRangeMode = lcrMenu.add("Range", &rangeModeLabelSelection);
  lcrmenuBtnCorr = lcrMenu.add("Corr.");
  lcrmenuBtnSetAvg = lcrMenu.add("Avg.", &avgStrPtr);
  lcrmenuBtnDisplMode = lcrMenu.add("Display", &displModeLabelSelection);

  lcrSetFreqMenu.init(btn_feedback, "Set Frequency");
  lcrmenuBtnF100 = lcrSetFreqMenu.add("100Hz");
  lcrmenuBtnF1k = lcrSetFreqMenu.add("1kHz");
  lcrmenuBtnF10k = lcrSetFreqMenu.add("10kHz");
  lcrmenuBtnFman = lcrSetFreqMenu.add("Manuell");

  lcrResetScreen();
  
  // loop
  while (1)
  {
    lcrHandleButtons();
    if (lcrHandleTouch())
      return;
    adAverageReadings();

    if (displayUpdate >= DISPLAY_UPDATE_RATE_MIN && adDataAvailable)
    {
      adDataAvailable = false;
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
