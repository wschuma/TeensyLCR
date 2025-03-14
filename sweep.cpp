#include <ili9341_t3n_font_Arial.h>
#include "audio_design.h"
#include "autorange.h"
#include "board.h"
#include "correction.h"
#include "displayhelp.h"
#include "globals.h"
#include "helper.h"
#include "lcr_param.h"
#include "lcr_setup.h"
#include "src/utils/btn_bar_menu.h"
#include "src/utils/osdmessage.h"
#include "sweep.h"

static const uint8_t SWEEP_DATA_POINTS_COUNT = 100;

typedef struct sweep_settings_struct {
  float start;
  float stop;
  bool logarithmic;
  uint8_t function;
  bool graph;
} sweep_settings_t;

sweep_settings_t sweepSettings = {
  .start = LCR_MIN_FREQUENCY,
  .stop = LCR_MAX_FREQUENCY,
  .logarithmic = true,
  .function = 0,
  .graph = false,
};

typedef struct sweep_results_struct {
  float freq[SWEEP_DATA_POINTS_COUNT];
  float prim[SWEEP_DATA_POINTS_COUNT];
  float secn[SWEEP_DATA_POINTS_COUNT];
  uint8_t length;
  uint8_t count;
} sweep_results_t;

sweep_results_t sweepResults  = {
  .length = 0,
  .count = 0,
};

BtnBarMenu sweepMenu(&tft);

const char *viewLabels[] = {"Table", "Graph"};
const char *viewLabelSelection;

const char *linLogLabels[] = {"Lin.", "Log."};
const char *linLogLabelSelection;

const uint8_t TABLE_MAX_ROWS = 5;
uint8_t resultPage = 0;

/*
 * Calculate frequency points according to sweep settings.
 */
void setupMeasArray()
{
  float tmp;
  float factor = log(sweepSettings.stop / sweepSettings.start) / log(10) / (SWEEP_DATA_POINTS_COUNT - 1);
  uint8_t no = 0;
  
  for (uint8_t point = 0; point < SWEEP_DATA_POINTS_COUNT; point++)
  {
    if (sweepSettings.logarithmic)
      tmp = sweepSettings.start * pow(10, point * factor);
    else
      tmp = map(point, 0, SWEEP_DATA_POINTS_COUNT - 1, sweepSettings.start, sweepSettings.stop);
    // rounding
    tmp = (tmp + LCR_FREQ_RESOLUTION / 2) / LCR_FREQ_RESOLUTION;
    sweepResults.freq[no] = (long)tmp * LCR_FREQ_RESOLUTION;
    // skip equal frequencies
    if (no > 0 && sweepResults.freq[no - 1] == sweepResults.freq[no])
      continue;
    if (sweepResults.freq[no] > 90000)
      break;
    // reset value
    sweepResults.prim[no] = 0;
    sweepResults.secn[no] = 0;
    no++;
  }
  sweepResults.length = no;
}

/*
 * Draw result table to screen.
 */
void drawResultTable()
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
  tft.print(lcrParams[lcrSettings.function][0]->label);
  if (lcrParams[lcrSettings.function][0]->unit[0] > 0)
  {
    tft.print(" [");
    tft.print(lcrParams[lcrSettings.function][0]->unit);
    tft.print(']');
  }
  // secondary function
  tft.setCursor(225, tft.getCursorY());
  tft.print(lcrParams[lcrSettings.function][1]->label);
  if (lcrParams[lcrSettings.function][1]->unit[0] > 0)
  {
    tft.print(" [");
    tft.print(lcrParams[lcrSettings.function][1]->unit);
    tft.println(']');
  }
  else
  {
    tft.println();
  }
  // draw values
  tft.setTextColor(ILI9341_WHITE);
  for (uint8_t row = 0; row < TABLE_MAX_ROWS; row++)
  {
    uint8_t point = resultPage * TABLE_MAX_ROWS + row;
    if (point >= sweepResults.count)
      break;
    tft.setCursor(0, tft.getCursorY());
    tft.print(point + 1);
    tft.setCursor(50, tft.getCursorY());
    tft.print((long)sweepResults.freq[point]);

    tft.setCursor(125, tft.getCursorY());
    value = sweepResults.prim[point];
    resolution = lcrParams[lcrSettings.function][0]->resolution;
    getDisplValue(value, 5, resolution, &val);
    if (val.minus)
      tft.print("-");
    tft.print(val.str);
    tft.print(" ");
    tft.print(val.modifier);

    tft.setCursor(225, tft.getCursorY());
    value = sweepResults.secn[point];
    resolution = lcrParams[lcrSettings.function][1]->resolution;
    getDisplValue(value, 5, resolution, &val);
    if (val.minus)
      tft.print("-");
    tft.print(val.str);
    tft.print(" ");
    tft.println(val.modifier);
  }
}

/*
 * Draw sweep display page to screen.
 */
void sweepDrawPage()
{
  tft.fillRect(0, 0, 320, tft.height() - 69, ILI9341_BLACK);
  tft.setFont(Arial_14);
  
  // print sweep parameter
  // draw labels
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_GREEN);
  tft.println(F("START:"));
  tft.println(F("STOP:"));
  tft.setCursor(170, 0);
  tft.println(F("SCALE:"));
  tft.setCursor(170, tft.getCursorY());
  tft.println();

  // draw values
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(75, 0);
  if (sweepSettings.start < 1000) {
    tft.print(sweepSettings.start, 0);
    tft.println(" Hz");
  } else {
    tft.print(sweepSettings.start / 1000, 2);
    tft.println(" kHz");
  }

  tft.setCursor(75, tft.getCursorY());
  if (sweepSettings.stop < 1000) {
    tft.print(sweepSettings.stop, 0);
    tft.println(" Hz");
  } else {
    tft.print(sweepSettings.stop / 1000, 2);
    tft.println(" kHz");
  }

  tft.setCursor(245, 0);
  tft.println(linLogLabelSelection);
  tft.setCursor(245, tft.getCursorY());
  tft.println();

  drawResultTable();
}

/*
 * Save sweep data as text file to USB drive.
 */
void saveSweepDataToFile()
{
  int state = usbCheckPartition();
  if (state) {
    return;
  }
  
  // find filename
  char filePath[20];
  uint filenr = 0;
  do {
    if (++filenr > 9999) {
      return;
    }
    sprintf(filePath, "/sweepdata_%04i.txt", filenr);
  } while (usbPartition1.exists(filePath));

  File f = usbPartition1.open(filePath, FILE_WRITE_BEGIN);
  if (!f) {
    osdMessage.setMessage(F("Failed to create file!"));
    return;
  }

  // set timestamp of file
  DateTimeFields dtf;
  breakTime(Teensy3Clock.get(), dtf);
  f.setCreateTime(dtf);
  f.setModifyTime(dtf);

  // write data to file
  char buf[15];
  f.println(F("SWEEP MEASUREMENT"));
  f.println();
  f.print(F("Date:\t"));
  sprintf(buf, "%04i-%02i-%02i", year(), month(), day());
  f.println(buf);
  f.print(F("Time:\t"));
  sprintf(buf, "%02i:%02i:%02i", hour(), minute(), second());
  f.println(buf);
  f.print(F("Start:\t"));
  f.print(sweepSettings.start, 0);
  f.println("\tHz");
  f.print(F("Stop:\t"));
  f.print(sweepSettings.stop, 0);
  f.println("\tHz");
  f.print(F("Scale:\t"));
  f.println(linLogLabelSelection);
  f.print(F("Level:\t"));
  f.print(lcrSettings.level, 3);
  f.println("\tV");
  f.print(F("No.\tFrequency\t"));
  f.print(lcrParams[lcrSettings.function][0]->label);
  f.print('\t');
  f.println(lcrParams[lcrSettings.function][1]->label);
  for (uint8_t point = 0; point < sweepResults.count; point++)
  {
    f.print(point + 1);
    f.print('\t');
    f.print(sweepResults.freq[point], 0);
    f.print('\t');
    sprintf(buf, "%E", sweepResults.prim[point]);
    f.print(buf);
    f.print('\t');
    sprintf(buf, "%E", sweepResults.secn[point]);
    f.println(buf);
  }
  f.close();
}

/*
 * Run sweep measurement.
 */
void runSweepMeas()
{
  TS_Point p;
  BtnBarMenu abortMenu(&tft);
  abortMenu.init(btn_feedback);
  int abortBtn = abortMenu.add("Abort");
  abortMenu.draw();

  // use setup from regular lcr measurements (function, level and averaging)
  setupMeasArray();
  resultPage = 0;
  sweepResults.count = 0;
  uint lcrAveraging = adGetMinAveraging();
  bool forceRanging;
  uint8_t measToDo;
  bool aborted = false;
  bool applyCorrection = lcrSettings.applyCorrection && (corr_data.ts_open > 0 || corr_data.ts_short > 0);

  osdMessage.setMessage(F("Measurement in progress..."));
  osdMessage.show();
  tft.updateScreen();

  // run sweep
  for (uint8_t point = 0; point < sweepResults.length; point++)
  {
    float f = sweepResults.freq[point];
    adSetOutputFrequency(f);
    forceRanging = true;
    measToDo = 1;

    drawProgressBar((float)point / sweepResults.length);
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
          adSetMinAveraging(lcrAveraging);
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

    // store data
    float impedance = adReadings.v_rms / adReadings.i_rms;
    float phase = adReadings.phase;

    if (applyCorrection)
      corrApply(&impedance, &phase, f);

    Complex z;
    z.polar(impedance, phase);
    sweepResults.prim[point] = lcrParams[lcrSettings.function][0]->value(z, f);
    sweepResults.secn[point] = lcrParams[lcrSettings.function][1]->value(z, f);
    sweepResults.count++;
  }
  
  if (aborted)
    osdMessage.setMessage(F("Measurement aborted."));
  else
    osdMessage.clear();

  // save measured data to flash drive, if available.
  if (sweepResults.count)
    saveSweepDataToFile();

  sweepDrawPage();
  sweepMenu.draw();
  osdMessage.show();
  tft.updateScreen();
}

void setStartFreq()
{
  float f = enterFrequency(LCR_MIN_FREQUENCY, sweepSettings.stop - LCR_FREQ_RESOLUTION, "Enter start frequency:");
  f = f / LCR_FREQ_RESOLUTION; // set last digit to 0
  f = (uint)round(f) * LCR_FREQ_RESOLUTION;
  if (f > 0)
  {
    sweepSettings.start = f;
    sweepResults.count = 0;
  }
  
  sweepDrawPage();
  sweepMenu.draw();
  tft.updateScreen();
}

void setStopFreq()
{
  float f = enterFrequency(sweepSettings.start + LCR_FREQ_RESOLUTION, LCR_MAX_FREQUENCY, "Enter stop frequency:");
  f = f / LCR_FREQ_RESOLUTION; // set last digit to 0
  f = (uint)round(f) * LCR_FREQ_RESOLUTION;
  if (f > 0)
  {
    sweepSettings.stop = f;
    sweepResults.count = 0;
  }
  
  sweepDrawPage();
  sweepMenu.draw();
  tft.updateScreen();
}

void toggleSweepScale()
{
  sweepSettings.logarithmic = !sweepSettings.logarithmic;
  if (sweepSettings.logarithmic) {
    linLogLabelSelection = linLogLabels[1];
  } else {
    linLogLabelSelection = linLogLabels[0];
  }
  sweepResults.count = 0;
  
  sweepDrawPage();
  sweepMenu.draw();
  tft.updateScreen();
}

void toggleView()
{
  /*sweepSettings.graph = !sweepSettings.graph;
  if (sweepSettings.graph) {
    viewLabelSelection = viewLabels[1];
  } else {
    viewLabelSelection = viewLabels[0];
  }
  
  sweepMenu.draw();
  tft.updateScreen();*/
}

void decResultPage()
{
  if (sweepResults.count == 0)
    return;
  if (resultPage == 0)
    resultPage = (sweepResults.count - 1) / TABLE_MAX_ROWS;
  else
    resultPage--;
  
  sweepDrawPage();
  tft.updateScreen();
}

void incResultPage()
{
  if (sweepResults.count == 0)
    return;
  uint8_t maxPages = 1 + (sweepResults.count - 1) / TABLE_MAX_ROWS;
  resultPage = ++resultPage % maxPages;
  
  sweepDrawPage();
  tft.updateScreen();
}

void sweepPage()
{
  if (sweepSettings.graph) {
    viewLabelSelection = viewLabels[1];
  } else {
    viewLabelSelection = viewLabels[0];
  }
  if (sweepSettings.logarithmic) {
    linLogLabelSelection = linLogLabels[1];
  } else {
    linLogLabelSelection = linLogLabels[0];
  }
  if (lcrSettings.function != sweepSettings.function)
  {
    sweepResults.count = 0;
    sweepSettings.function = lcrSettings.function;
  }

  sweepDrawPage();

  sweepMenu.init(btn_feedback, "List Sweep Menu");
  int sweepMenuBtnExit = sweepMenu.add("Exit");
  sweepMenu.add("Run", runSweepMeas);
  sweepMenu.add("Scale", &linLogLabelSelection, toggleSweepScale);
  sweepMenu.add("Start", setStartFreq);
  sweepMenu.add("Stop", setStopFreq);
  //sweepMenu.add("View", &viewLabelSelection, toggleView);
  sweepMenu.draw();
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
        decResultPage();
        break;
      case 'R':
        incResultPage();
        break;
      default:
        break;
    }

    if (getTouchPoint(&p))
    {
      key = sweepMenu.processTSPoint(p);
      if (key == sweepMenuBtnExit)
        break;
    }
    
    if (osdMessage.clean())
    {
      sweepDrawPage();
      tft.updateScreen();
    }
  }

  osdMessage.clear();
}
