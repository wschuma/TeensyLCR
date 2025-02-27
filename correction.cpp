#include <ili9341_t3n_font_Arial.h>
#include <TimeLib.h>
#include "audio_design.h"
#include "autorange.h"
#include "board.h"
#include "correction.h"
#include "displayhelp.h"
#include "helper.h"
#include "src/utils/btn_bar_menu.h"
#include "src/utils/osdmessage.h"

corr_data_t corr_data;
bool corr_apply;

BtnBarMenu menuCorr(&tft);

const char *offOnApplyLabels[] = {"OFF", "ON"};
const char *corrApplyLabelSelection;

const uint8_t TABLE_MAX_ROWS = 5;
uint8_t corrResultPage = 0;

bool corrInterpolation(Complex *zs, Complex *zp, float frequency)
{
  if (frequency < corr_frequencies[0])
    return false;
  
  for (uint8_t point = 0; point < CORR_FREQ_COUNT; point++)
  {
    if (corr_frequencies[point] == frequency)
    {
      *zs = corr_data.zs[point];
      *zp = corr_data.zp[point];
      return true;
    }
    if (corr_frequencies[point] > frequency)
    {
      float df = corr_frequencies[point] - corr_frequencies[point - 1];
      Complex m = (corr_data.zs[point] - corr_data.zs[point - 1]) / df;
      *zs = m * (frequency - corr_frequencies[0]) + corr_data.zs[point - 1];
      m = (corr_data.zp[point] - corr_data.zp[point - 1]) / df;
      *zp = m * (frequency - corr_frequencies[0]) + corr_data.zp[point - 1];
      return true;
    }
  }
  return false;
}

/*
 * Use correction parameters to calculate impedance.
 */
void corrApply(float *impedance, float *phase, float frequency)
{
  Complex zs, zp;
  if (!corrInterpolation(&zs, &zp, frequency))
    return;
  Complex zm, zdut, tmp;
  zm.polar(*impedance, *phase);
  tmp = zm - zs;
  zdut = tmp / (-(tmp / zp) + 1);
  *impedance = zdut.modulus();
  *phase = zdut.phase();
}

/*
 * Draw result table to screen.
 */
void drawCorrTable()
{
  disp_val_t val;
  float value;
  int8_t resolution;
  tft.setCursor(0, tft.getCursorY());
  // draw header
  // sweep point no.
  tft.setTextColor(ILI9341_YELLOW);
  tft.print("No.");
  // frequency
  tft.setCursor(50, tft.getCursorY());
  tft.print(F("F [Hz]"));
  // primary function
  tft.setCursor(125, tft.getCursorY());
  tft.print("Zs");
  // secondary function
  tft.setCursor(225, tft.getCursorY());
  tft.println("Phi");
  // draw values
  tft.setTextColor(ILI9341_WHITE);
  if (corr_data.ts_short == 0)
    return;
  for (uint8_t row = 0; row < TABLE_MAX_ROWS; row++)
  {
    uint8_t point = corrResultPage * TABLE_MAX_ROWS + row;
    if (point >= CORR_FREQ_COUNT)
      break;
    tft.setCursor(0, tft.getCursorY());
    tft.print(point + 1);
    tft.setCursor(50, tft.getCursorY());
    tft.print((long)corr_frequencies[point]);

    tft.setCursor(125, tft.getCursorY());
    value = corr_data.zs[point].modulus();
    resolution = -5;
    getDisplValue(value, 5, resolution, &val);
    if (val.minus)
      tft.print("-");
    tft.print(val.str);
    tft.print(" ");
    tft.print(val.modifier);

    tft.setCursor(225, tft.getCursorY());
    value = corr_data.zs[point].phase() * 180 / PI;
    resolution = -3;
    getDisplValue(value, 5, resolution, &val);
    if (val.minus)
      tft.print("-");
    tft.print(val.str);
    tft.print(" ");
    tft.println(val.modifier);
  }
}

void corrDrawPage()
{
  tft.fillRect(0, 0, 320, tft.height() - 69, ILI9341_BLACK);
  tft.setFont(Arial_14);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(0, 0);
  
  // print date and time of last open correction measurement
  tft.print("Open cal:");
  tft.setCursor(90, tft.getCursorY());
  if (corr_data.ts_open > 0) {
    printDateTime(corr_data.ts_open);
    tft.println();
  } else {
    tft.println("N/A");
  }

  // print date and time of last short correction measurement
  tft.print("Short cal:");
  tft.setCursor(90, tft.getCursorY());
  if (corr_data.ts_short > 0) {
    printDateTime(corr_data.ts_short);
    tft.println();
  } else {
    tft.println("N/A");
  }

  if (corr_apply) {
    corrApplyLabelSelection = offOnApplyLabels[1];
  } else {
    corrApplyLabelSelection = offOnApplyLabels[0];
  }

  drawCorrTable();
}

void corrDecResultPage()
{
  if (corr_data.ts_short == 0)
    return;
  if (corrResultPage == 0)
    corrResultPage = (CORR_FREQ_COUNT - 1) / TABLE_MAX_ROWS;
  else
    corrResultPage--;
  
  corrDrawPage();
  tft.updateScreen();
}

void corrIncResultPage()
{
  if (corr_data.ts_short == 0)
    return;
  uint8_t maxPages = 1 + (CORR_FREQ_COUNT - 1) / TABLE_MAX_ROWS;
  corrResultPage = ++corrResultPage % maxPages;
  
  corrDrawPage();
  tft.updateScreen();
}

/*
 * Run correction measurement.
 * If `open` = true, runs open correction. Otherwise runs short correction.
 */
void corrRunMeas(bool open)
{
  // show instruction message
  if (open) {
    showMessage(F("Open-circuit the test terminals."));
  } else {
    showMessage(F("Short-circuit the test terminals."));
  }
  
  const float OPEN_MEAS_MIN_IMPEDANCE = 1e6;
  const float SHORT_MEAS_MAX_IMPEDANCE = 1.0;

  TS_Point p;
  BtnBarMenu abortMenu(&tft);
  abortMenu.init(btn_feedback);
  int abortBtn = abortMenu.add("Abort");
  abortMenu.draw();

  // setup correction
  adSetOutputAmplitude(1.0 * sqrtf(2));
  adSetOutputOffset(0);
  bool forceRanging;
  uint8_t measToDo;
  bool aborted = false;

  osdMessage.setMessage(F("Measurement in progress..."));
  osdMessage.show();
  tft.updateScreen();

  // run sweep
  for (uint8_t point = 0; point < CORR_FREQ_COUNT; point++)
  {
    float f = corr_frequencies[point];
    adSetOutputFrequency(f);
    forceRanging = true;
    measToDo = 1;
    
    drawProgressBar((float)point / CORR_FREQ_COUNT);
    tft.updateScreen();

    // take readings
    while (1) {
      if (getTouchPoint(&p))
      {
        if (abortMenu.processTSPoint(p) == abortBtn) {
          aborted = true;
          break;
        }
      }

      adAverageReadings();
      if (adDataAvailable) {
        RangingState state = autoRange(false, forceRanging);
        if (state == RangingState::Started) {
          // ranging started
          forceRanging = false;
        } else if (state == RangingState::Finished) {
          // finished ranging, restore averaging
          adSetAveraging(128);
        } else if (state == RangingState::None) {
          // range is ok and readings are available
          if (measToDo-- == 0)
            break;
          adDataAvailable = false;
        }
      }
    }
    if (aborted)
      break;
    
    float impedance = adReadings.v_rms / adReadings.i_rms;
    float phase = adReadings.phase;

    // check readings
    if (open && impedance < OPEN_MEAS_MIN_IMPEDANCE)
    {
      osdMessage.setMessage(F("Error! Terminals not open!"));
      corr_data.ts_open = 0;
      corr_data.ts_short = 0;
      corr_apply = false;
      return;
    }
    else if (!open && impedance > SHORT_MEAS_MAX_IMPEDANCE)
    {
      osdMessage.setMessage(F("Error! Terminals not shorted!"));
      corr_data.ts_open = 0;
      corr_data.ts_short = 0;
      corr_apply = false;
      return;
    }

    // store data
    if (open) {
      corr_data.z0[point].polar(impedance, phase);
      if (corr_data.ts_short == 0)
        corr_data.zs[point] = 0;
    } else {
      corr_data.zs[point].polar(impedance, phase);
      if (corr_data.ts_open == 0)
        corr_data.z0[point] = one * 9.9e37;
    }
    corr_data.zp[point] = corr_data.z0[point] - corr_data.zs[point];
  }

  if (aborted) {
    osdMessage.setMessage(F("Measurement aborted."));
    corr_data.ts_open = 0;
    corr_data.ts_short = 0;
    corr_apply = false;
    return;
  }

  osdMessage.clear();

  if (open) {
    corr_data.ts_open = now();
  } else {
    corr_data.ts_short = now();
  }
}

void corrRunOpenMeas()
{
  corrRunMeas(true);
  corrDrawPage();
  menuCorr.draw();
  osdMessage.show();
  tft.updateScreen();
}

void corrRunShortMeas()
{
  corrRunMeas(false);
  corrDrawPage();
  menuCorr.draw();
  osdMessage.show();
  tft.updateScreen();
}

void corrToggleApply()
{
  if (corr_data.ts_open == 0 && corr_data.ts_short == 0)
    return;
  corr_apply = !corr_apply;
  if (corr_apply) {
    corrApplyLabelSelection = offOnApplyLabels[1];
  } else {
    corrApplyLabelSelection = offOnApplyLabels[0];
  }
  menuCorr.draw();
  tft.updateScreen();
}

void correctionMenu()
{
  corrDrawPage();

  menuCorr.init(btn_feedback, "Correction Menu");
  int menuCorrBtnExit = menuCorr.add("Exit");
  menuCorr.add("Apply", &corrApplyLabelSelection, corrToggleApply);
  menuCorr.add("Open", corrRunOpenMeas);
  menuCorr.add("Short", corrRunShortMeas);
  menuCorr.draw();
  tft.updateScreen();

  char key;
  TS_Point p;
  while(1)
  {
    key = keypad.getKey();
    switch (key) {
      case 'S':
        saveScreenshot(&tft);
        osdMessage.show();
        tft.updateScreen();
        break;
      case 'L':
        corrDecResultPage();
        break;
      case 'R':
        corrIncResultPage();
        break;
      default:
        break;
    }

    if (getTouchPoint(&p))
    {
      key = menuCorr.processTSPoint(p);
      if (key == menuCorrBtnExit)
        break;
    }
    
    if (osdMessage.clean())
    {
      corrDrawPage();
      tft.updateScreen();
    }
  }

  osdMessage.clear();
}
