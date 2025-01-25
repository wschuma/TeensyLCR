#include "apps.h"
#include <Arduino.h>
#include "audio_design.h"
#include "autorange.h"
#include "board.h"
#include "CComplex.h"
#include "correction.h"
#include "displayhelp.h"
#include "func_lcr.h"
#include "globals.h"
#include "helper.h"
#include <ili9341_t3n_font_Arial.h>
#include "lcr_param.h"
#include "lcr_setup.h"
#include "MathHelpers.h"
#include "settings.h"
#include "src/utils/btn_bar_menu.h"
#include "sweep.h"
#include "sysmenu.h"

static const uint8_t PRIMARY = 0;
static const uint8_t SECONDARY = 1;

bool forceRanging = false;

// Menu definition
BtnBarMenu lcrMenu(&tft);
BtnBarMenu lcrSetFreqMenu(&tft);

const char *functionLabelSelection;

const char *displModeLabels[] = {"Normal", "Debug"};
const char *displModeLabelSelection;

const char *rangeModeLabels[] = {"Auto", "Hold"};
const char *rangeModeLabelSelection;

const char *offOnLabels[] = {"OFF", "ON"};
const char *corrLabelSelection;
bool applyCorrection = false;

char avgStr[6];

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

void lcrDrawFuncIndicators()
{
  // skip if debug display
  if (lcrSettings.displMode != 0)
    return;

  tft.fillRect(0, MAIN_DISPLAY_Y_PRIMARY, 31, 14, ILI9341_BLACK);
  tft.setFont(Arial_14);
  tft.setTextColor(ILI9341_GREEN);
  tft.setCursor(0, MAIN_DISPLAY_Y_PRIMARY);
  tft.print(lcrParams[lcrSettings.function][PRIMARY]->label);

  tft.fillRect(0, MAIN_DISPLAY_Y_SECONDARY, 31, 14, ILI9341_BLACK);
  tft.setCursor(0, MAIN_DISPLAY_Y_SECONDARY);
  tft.print(lcrParams[lcrSettings.function][SECONDARY]->label);
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

  if (applyCorrection)
    corrApply(&impedance, &phase);

  if (impedance > LCR_THRESHOLD_OPEN)
  {
    drawPrimaryDisplay(" Open");
    drawSecondaryDisplay("");
    return;
  }

  Complex z;
  z.polar(impedance, phase);
  disp_val_t val;
  
  float value = lcrParams[lcrSettings.function][PRIMARY]->value(z, lcrSettings.frequency);
  int8_t resolution = lcrParams[lcrSettings.function][PRIMARY]->resolution;
  getDisplValue(value, 5, resolution, &val);
  val.unit = lcrParams[lcrSettings.function][PRIMARY]->unit;
  drawPrimaryDisplay(&val);
  
  value = lcrParams[lcrSettings.function][SECONDARY]->value(z, lcrSettings.frequency);
  resolution = lcrParams[lcrSettings.function][SECONDARY]->resolution;
  getDisplValue(value, 5, resolution, &val);
  val.unit = lcrParams[lcrSettings.function][SECONDARY]->unit;
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
  Complex z;
  z.polar(impedance, phase);
  
  if (applyCorrection)
    corrApply(&impedance, &phase);

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
  tft.println(sci(z.modulus(), 6));
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
    forceRanging = true;
    lcrUpdateCorrState();
  }
  activeMenu = APP_DEFAULT;
  lcrResetScreen();
}

void lcrShowSelectFreqMenu()
{
  activeMenu = LCR_SET_FREQUENCY;
  lcrDrawMenu();
}

void lcrSetFreq100()
{
  lcrSetFrequency(100.0);
}

void lcrSetFreq1k()
{
  lcrSetFrequency(1000.0);
}

void lcrSetFreq10k()
{
  lcrSetFrequency(10000.0);
}

void lcrSetFreqMan()
{
  lcrSetFrequency(0);
}

void lcrSetAmplitude()
{
  lcrSettings.amplitudePreset = ++lcrSettings.amplitudePreset % AMPLITUDE_PRESETS_NUM;
  adSetOutputAmplitude(amplitudePresets[lcrSettings.amplitudePreset] * sqrtf(2));
  forceRanging = true;
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
    lcrMenu.processTSPoint(p);
    return false;
  }
  else if (activeMenu == SELECT_FUNCTION) {
    key = appSelectMenu.processTSPoint(p);
    if (key != BTN_BAR_MENU_EVENT_IDLE) {
      appId = key;
      return true;
    }
  }
  else if (activeMenu == LCR_SET_FREQUENCY) {
    lcrSetFreqMenu.processTSPoint(p);
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
  lcrMenu.add("Freq.", lcrShowSelectFreqMenu);
  lcrMenu.add("Level", lcrSetAmplitude);
  lcrMenu.add("Function", &functionLabelSelection, lcr_select_func);
  lcrMenu.add("Range", &rangeModeLabelSelection, lcrSetRangeMode);
  lcrMenu.add("Corr.", lcrCorrectionMenu);
  lcrMenu.add("Avg.", &avgStrPtr, lcrSetAveraging);
  lcrMenu.add("Display", &displModeLabelSelection, lcrSetDisplayMode);

  lcrSetFreqMenu.init(btn_feedback, "Set Frequency");
  lcrSetFreqMenu.add("100Hz", lcrSetFreq100);
  lcrSetFreqMenu.add("1kHz", lcrSetFreq1k);
  lcrSetFreqMenu.add("10kHz", lcrSetFreq10k);
  lcrSetFreqMenu.add("Manuell", lcrSetFreqMan);

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
