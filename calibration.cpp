#include <Arduino.h>
#include "audio_design.h"
#include "board.h"
#include "calibration.h"
#include "displayhelp.h"
#include "globals.h"
#include <ili9341_t3n_font_Arial.h>
#include "settings.h"

calFactorOutput_t calOutA;
calFactorInputA_t calInA;
calFactorInputB_t calInB;

// Phase correction values
// Measured phase depends on gain and frequency
float calPhaseInputA[PGA_GAIN_NUM] = {0, 1.4879e-6, 4.3633e-8, -1.5054e-6};
float calPhaseInputB[PGA_GAIN_NUM] = {0, -1.4879e-6, -4.3633e-8, 1.8064e-6};

// init calibration data
void calInit()
{
  calOutA.offset = 0;
  calOutA.transmissionFactor = 0.21; // 1/V
  calOutA.gainFactor = 1.087;
  
  calInA.offset = 0;
  calInA.transmissionFactor = 4.95; // V/1
  calInA.gainFactor[0] = 1.079;
  calInA.gainFactor[1] = 0.1923;
  calInA.gainFactor[2] = 3.984e-2;
  calInA.gainFactor[3] = 9.96e-3;
  
  calInB.offset = 0;
  calInB.transmissionFactor[0] = 2.5e-2; // A/1
  calInB.transmissionFactor[1] = 2.5e-3;
  calInB.transmissionFactor[2] = 2.5e-4;
  calInB.transmissionFactor[3] = 2.5e-5;
  calInB.gainFactor[0] = 1.0;
  calInB.gainFactor[1] = 0.1923;
  calInB.gainFactor[2] = 3.984e-2;
  calInB.gainFactor[3] = 9.96e-3;
}

void calClearScreen()
{
  tft.fillScreen(ILI9341_BLACK);
  tft.setFont(Arial_14);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(0, 0);
}

void waitForUser()
{
  tft.println("Press key to continue...");
  tft.updateScreen();
  while (!keypad.getKey()) {
    delay(1);
  }
}

void updateReadings()
{
  uint8_t measToDo = 1;
  adDataAvailable = false;

  // take readings
  while (1) {
    adAverageReadings();
    if (adDataAvailable) {
      // readings are available
      adDataAvailable = false;
      if (measToDo-- == 0)
        break;
    }
  }
}

void calSaveData()
{
  calClearScreen();
  tft.println("Save data...");
  tft.updateScreen();

  AudioNoInterrupts();
  int result = saveCalibrationData();
  AudioInterrupts();

  if (result != 0) {
    tft.println("Failed to save data!");
    tft.println(result);
  } else {
    tft.println("done");
  }
  waitForUser();
}

void printCalData()
{
  char buf[25];
  
  Serial.println("calOutA");
  sprintf(buf, "%+E", calOutA.transmissionFactor);
  Serial.println(buf);
  sprintf(buf, "%+E", calOutA.gainFactor);
  Serial.println(buf);
  
  Serial.println("calInA");
  sprintf(buf, "%+E", calInA.transmissionFactor);
  Serial.println(buf);
  for (uint idx = 0; idx < PGA_GAIN_NUM; idx++)
  {
    sprintf(buf, "%+E", calInA.gainFactor[idx]);
    Serial.println(buf);
  }

  Serial.println("calInB");
  for (uint idx = 0; idx < PGA_GAIN_NUM; idx++)
  {
    sprintf(buf, "%+E", calInB.transmissionFactor[idx]);
    Serial.println(buf);
  }
  for (uint idx = 0; idx < PGA_GAIN_NUM; idx++)
  {
    sprintf(buf, "%+E", calInB.gainFactor[idx]);
    Serial.println(buf);
  }
}

void showCalMenu()
{
  calClearScreen();
  tft.println("[ CALIBRATION ]");
  tft.println("1 Adjust voltage output");
  tft.println("2 Adjust voltage input");
  tft.println("3 Adjust current input");
  tft.println("4 Print calibration data");
  tft.println("9 Save calibration data to EEPROM");
  tft.updateScreen();
}

void calOutput()
{
  // board setup
  board.setLCRRange(LCR_RANGE_100);
  adSetMinAveraging(50);
  calOutA.gainFactor = 1.0;
  float vOut = 1.0;
  adSetOutputAmplitude(vOut * sqrtf(2));
  adSetOutputOffset(0);
  adSetOutputFrequency(1000.0);

  // preparation
  calClearScreen();
  tft.println("[ ADJUST VOLTAGE OUTPUT ]");
  tft.println("Connect AC voltmeter to HCUR.");
  waitForUser();

  // adjust output
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.println("Enter measured AC voltage:");
  float vRef = enterFloat(0.5, 2, "Vrms", UNIT_PREFIX_NONE);
  float factor = vOut / vRef;
  calClearScreen();
  char buf[25];
  sprintf(buf, "%E", factor);
  tft.println(buf);

  // sanity check
  if (factor > 0.3 || factor < 3) {
    calOutA.gainFactor = factor;
    tft.println("Done.");
  } else {
    tft.println("Gain factor out of range!");
  }

  // turn off output
  adSetOutputAmplitude(0);

  waitForUser();
}

void calInputV()
{
  typedef struct cal_setup_struct {
    uint range;
    float level;
    uint calr;
  } cal_setup_t;
  cal_setup_t calSetups[] = {
    { 0, 2.2, 0 },
    { 3, 1.0, 100 },
    { 3, 1.0, 1000 },
    { 3, 2.2, 10000 },
  };
  const float rParallel = 110030.0; // series resistance of mux, 100k range resistor and 10k resistor between HCUR & HPOT

  // board setup
  board.setLCRRange(LCR_RANGE_100);
  board.setPGAGainV(PGA_GAIN_1);
  adSetMinAveraging(50);
  adSetOutputAmplitude(calSetups[0].level * sqrtf(2));
  adSetOutputOffset(0);
  adSetOutputFrequency(1000.0);

  // preparation
  calClearScreen();
  tft.println("[ ADJUST VOLTAGE INPUT ]");
  tft.println("Connect 4 wire test leads.");
  waitForUser();
  
  // calibrate voltage gain
  for (uint preset = 0; preset < PGA_GAIN_NUM; preset++)
  {
    board.setLCRRange(calSetups[preset].range);
    adSetOutputAmplitude(calSetups[preset].level * sqrtf(2));
    board.setPGAGainV(preset);
    calInA.gainFactor[board.getPGAGainV()] = 1.0;

    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(0, 0);
    tft.print("Adjust PGA V gain ");
    tft.println(preset);
    if (preset > 0) {
      tft.println("Connect calibration cable.");
      tft.print("Use ");
      tft.print(calSetups[preset].calr);
      tft.println("R cal resistor.");
    }
    waitForUser();
    float ra = calSetups[preset].calr * rParallel / (calSetups[preset].calr + rParallel);
    float rb = 100.0; // calibration cable resistor between HPOT & LPOT
    float vRef = calSetups[preset].level * rb / (ra + rb);
    // take readings
    calClearScreen();
    tft.println("measure...");
    tft.updateScreen();
    updateReadings();

    float factor = vRef / adReadings.v_rms;
    char buf[25];
    sprintf(buf, "%E", factor);
    tft.println(buf);

    tft.print("Input headroom: ");
    float hdr = adHeadroom(adReadings.v_peak);
    tft.print(hdr);
    tft.println(" dB");

    // sanity check
    if (hdr < 0.2 || hdr > 11)
    {
      tft.println("Headroom out of range!");
      break;
    } else {
      calInA.gainFactor[board.getPGAGainV()] = factor;
      waitForUser();
    }
  }

  // finished
  tft.println("Done.");
  waitForUser();
}

void calInputI()
{
  // calibrate input B (current)
  //calInB.transmissionFactor[0] = 2.5e-2; // fix
  typedef struct cal_setup_struct {
    uint range;
    uint gain;
    float calRmin;
    float calRmax;
    float level;
    bool calRange;
  } cal_setup_t;
  cal_setup_t calSetups[] = {
    { 0, 0, 90.0, 110.0, 1.4, false },
    { 0, 1, 560.0, 650.0, 3, false },
    { 0, 2, 2690.0, 3100.0, 2.8, false },
    { 1, 0, 990.0, 1100.0, 2.2, true },
    { 2, 0, 9900.0, 11000.0, 2.2, true },
    { 0, 3, 9900.0, 11000.0, 2.2, false },
    { 3, 0, 99000.0, 110000.0, 2.5, true }
  };

  // board setup
  board.setLCRRange(LCR_RANGE_100);
  board.setPGAGainV(PGA_GAIN_1);
  adSetMinAveraging(50);
  adSetOutputAmplitude(calSetups[0].level * sqrtf(2));
  adSetOutputOffset(0);
  adSetOutputFrequency(1000.0);

  // preparation
  calClearScreen();
  tft.println("[ ADJUST CURRENT INPUT ]");

  float current, calR;
  for (uint preset = 0; preset < 7; preset++)
  {
    board.setLCRRange(calSetups[preset].range);
    board.setPGAGainI(calSetups[preset].gain);
    adSetOutputAmplitude(calSetups[preset].level * sqrtf(2));
  
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(0, 0);
    tft.print("adjust preset ");
    tft.print(preset);
    tft.println(".");
    tft.updateScreen();
    if (preset == 0 || calSetups[preset].calRmax != calSetups[preset - 1].calRmax)
    {
      tft.print("Connect ");
      tft.print(calSetups[preset].calRmin, 0);
      tft.print("R to ");
      tft.print(calSetups[preset].calRmax, 0);
      tft.println("R.");
      tft.println("Use 4 wire connection.");
      tft.println("Enter exact value:");
      calR = enterFloat(calSetups[preset].calRmin, calSetups[preset].calRmax, "Ohm", UNIT_PREFIX_NONE | UNIT_PREFIX_KILO);
      calClearScreen();
      tft.println("measure...");
      tft.updateScreen();
    }
    if (calSetups[preset].calRange)
    {
      calInB.transmissionFactor[board.getLCRRange()] = 1.0;
    }
    else
    {
      calInB.gainFactor[board.getPGAGainI()] = 1.0;
    }
    updateReadings();
    current = adReadings.v_rms / calR;
    tft.print("I rms= ");
    char buf[25];
    sprintf(buf, "%E", current);
    tft.println(buf);
    // show headroom
    float hdrV = adHeadroom(adReadings.v_peak);
    float hdrI = adHeadroom(adReadings.i_peak);
    tft.print("h V= ");
    tft.print(hdrV);
    tft.print("dB I= ");
    tft.print(hdrI);
    tft.println("dB");
    // sanity check
    if (hdrV < 0.5 || hdrV > 16 || hdrI < 0.5 || hdrI > 14)
    {
      tft.println("Headroom out of range!");
      waitForUser();
      return;
    }
    // save values
    if (calSetups[preset].calRange)
    {
      calInB.transmissionFactor[board.getLCRRange()] = current / adReadings.i_rms;
      sprintf(buf, "%E", calInB.transmissionFactor[board.getLCRRange()]);
      tft.println(buf);
    }
    else
    {
      calInB.gainFactor[board.getPGAGainI()] = current / adReadings.i_rms;
      sprintf(buf, "%E", calInB.gainFactor[board.getPGAGainI()]);
      tft.println(buf);
    }
    waitForUser();
  }

  // finished
  tft.println("Done.");
  waitForUser();
}

void functionCalib()
{
  while (1)
  {
    showCalMenu();

    char key = 0;
    while (!key)
    {
      key = keypad.getKey();
      switch (key) {
        case '1':
          calOutput();
          break;
        case '2':
          calInputV();
          break;
        case '3':
          calInputI();
          break;
        case '4':
          printCalData();
          break;
        case '9':
          calSaveData();
          break;
        default:
          break;
      }
    }
  }
}
