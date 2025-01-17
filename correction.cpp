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

corr_data_t corr_data = {
  .z0 = one * 9.9e37,
  .z0_time = 0,
  .zs_time = 0,
  .f = 0,
  .apply = false,
};

BtnBarMenu menuCorr(&tft);

const char *offOnApplyLabels[] = {"OFF", "ON"};
const char *corrApplyLabelSelection;

/*
 * Use correction parameters to calculate impedance.
 */
void corrApply(float *impedance, float *phase)
{
  Complex zm, zdut, tmp;
  zm.polar(*impedance, *phase);
  tmp = zm - corr_data.zs;
  zdut = tmp / (-(tmp / corr_data.zp) + 1);
  *impedance = zdut.modulus();
  *phase = zdut.phase();
}

void corrDrawPage()
{
  tft.fillRect(0, 0, 320, tft.height() - 69, ILI9341_BLACK);
  tft.setFont(Arial_14);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(0, 0);
  tft.println("[ CORRECTION ]");
  
  // print date and time of last open correction measurement
  tft.println("Open");
  tft.print("Last cal:");
  tft.setCursor(80, tft.getCursorY());
  if (corr_data.z0_time > 0) {
    printDateTime(corr_data.z0_time);
    tft.println();
  } else {
    tft.println("N/A");
  }

  // print date and time of last short correction measurement
  tft.println("Short");
  tft.print("Last cal:");
  tft.setCursor(80, tft.getCursorY());
  if (corr_data.zs_time > 0) {
    printDateTime(corr_data.zs_time);
    tft.println();
  } else {
    tft.println("N/A");
  }

  // print correction parameter
  tft.print("G:");
  tft.setCursor(80, tft.getCursorY());
  tft.print(corr_data.z0.reciprocal().modulus() * 1.0e6, 5);
  tft.println(" uS");
  tft.print("R:");
  tft.setCursor(80, tft.getCursorY());
  tft.print(corr_data.zs.modulus(), 5);
  tft.println(" Ohm");
  tft.print("Spot:");
  tft.setCursor(80, tft.getCursorY());
  if (corr_data.f > 0)
  {
    tft.print(corr_data.f, 0);
    tft.println(" Hz");
  } else {
    tft.println("N/A");
  }

  if (corr_data.apply) {
    corrApplyLabelSelection = offOnApplyLabels[1];
  } else {
    corrApplyLabelSelection = offOnApplyLabels[0];
  }
}

/*
 * Run correction measurement.
 * If `open` = true, runs open correction. Otherwise runs short correction.
 */
void corrRunMeas(bool open)
{
  // show instruction message
  if (open) {
    showMessage("Open-circuit the test terminals.");
  } else {
    showMessage("Short-circuit the test terminals.");
  }
  
  TS_Point p;
  BtnBarMenu abortMenu(&tft);
  abortMenu.init(btn_feedback);
  int abortBtn = abortMenu.add("Abort");
  abortMenu.draw();

  if (corr_data.f != adGetFrequency()) {
    // Frequency not match. Clear previous correction data.
    corr_data.z0_time = 0;
    corr_data.zs_time = 0;
    corr_data.z0.set(9.9e37, 0);
    corr_data.zs.set(0, 0);
    corr_data.zp = corr_data.z0;
  }
  
  osdMessage.setMessage("Measurement in progress...");
  osdMessage.show();
  tft.updateScreen();

  // setup correction
  adSetOutputAmplitude(1.0 * sqrtf(2));
  adSetOutputOffset(0);
  adSetAveraging(256);
  bool forceRanging = true;

  // take readings
  while (1) {
    if (getTouchPoint(&p))
    {
      if (menuCorr.processTSPoint(p) == abortBtn) {
        osdMessage.clear();
        return;
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
        adSetAveraging(256);
      } else if (state == RangingState::None) {
        // range is ok and readings are available
        break;
      }
    }
  }
  
  float impedance = adReadings.v_rms / adReadings.i_rms;
  float phase = adReadings.phase;

  // check readings
  const float OPEN_MEAS_MIN_IMPEDANCE = 100e6;
  const float SHORT_MEAS_MAX_IMPEDANCE = 1.0;
  if (open && impedance < OPEN_MEAS_MIN_IMPEDANCE)
  {
    osdMessage.setMessage("Error! Terminals not open!");
    return;
  }
  else if (!open && impedance > SHORT_MEAS_MAX_IMPEDANCE)
  {
    osdMessage.setMessage("Error! Terminals not shorted!");
    return;
  }

  // store data
  corr_data.f = adGetFrequency();
  if (open) {
    corr_data.z0.polar(impedance, phase);
    corr_data.z0_time = now();
  } else {
    corr_data.zs.polar(impedance, phase);
    corr_data.zs_time = now();
  }
  corr_data.zp = corr_data.z0 - corr_data.zs;
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
  corr_data.apply = !corr_data.apply;
  if (corr_data.apply) {
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
    if (keypad.getKey() == 'S')
    {
      saveScreenshot(&tft);
      osdMessage.show();
      tft.updateScreen();
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
