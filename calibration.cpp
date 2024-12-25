#include "calibration.h"
#include <Arduino.h>
#include "globals.h"
#include "audio_design.h"
#include "board.h"
#include "settings.h"
#include "displayhelp.h"

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
  while(1)
  {
    adAverageReadings();
    if (adDataAvailable)
    {
      adDataAvailable = false;
      return;
    }
  }
}

void calSaveData()
{
  AudioNoInterrupts();
  tft.println("Save data...");
  int result = saveCalibrationData();
  if (result != 0) {
    tft.println("Failed to save data!");
    tft.println(result);
  } else {
    tft.println("done");
  }
  AudioInterrupts();
}

void functionCalib()
{
  // setup
  adSetAveraging(256);
  tft.useFrameBuffer(false);
  tft.fillScreen(ILI9341_BLACK);
  tft.setFont();
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(0, 0);
  tft.println("CALIBRATION");
  tft.updateScreen();

  // calibrate Vout
  boardSetLCRRange(LCR_RANGE_100);
  boardSetPGAGainV(PGA_GAIN_1);
  boardSetPGAGainI(PGA_GAIN_1);
  float vOut[] = {1.56, 0.32, 0.063, 0.015};
  calOutA.gainFactor = 1.0;
  adSetOutputAmplitude(vOut[0] * sqrtf(2));
  adSetOutputOffset(0);
  adSetOutputFrequency(100.0);
  tft.println("Connect HCUR to HPOT.");
  tft.println("Connect AC Voltmeter to");
  tft.println("HCUR/HPOT.");
  tft.println("Short LPOT.");
  waitForUser();

  // calibrate output
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.println("Enter measured AC voltage");
  tft.println("(V rms):");
  float vRef = enterFloat(6, false);
  calOutA.gainFactor = vOut[0] / vRef;
  tft.fillScreen(ILI9341_BLACK);
  tft.setFont();
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(0, 0);
  tft.println(calOutA.gainFactor, 6);

  // calibrate input A (voltage)
  //calInA.transmissionFactor = 2.5; // fix
  calInA.gainFactor[boardSettings.gain_v] = 1.0;
  tft.println("measure...");
  updateReadings();
  updateReadings();
  calInA.gainFactor[boardSettings.gain_v] = vRef / adReadings.v_rms;
  tft.println(calInA.gainFactor[boardSettings.gain_v], 6);
  
  
  // calibrate voltage gain
  for (uint preset = 1; preset < PGA_GAIN_NUM; preset++)
  {
    tft.print("adjust PGA V gain ");
    tft.print(preset);
    tft.println("...");
    tft.updateScreen();
    adSetOutputAmplitude(vOut[preset] * sqrtf(2));
    boardSetPGAGainV(preset);
    calInA.gainFactor[boardSettings.gain_v] = 1.0;
    updateReadings();
    updateReadings();
    tft.print("h V= ");
    tft.println(adHeadroom(adReadings.v_peak));
    calInA.gainFactor[boardSettings.gain_v] = vOut[preset] / adReadings.v_rms;
    tft.println(calInA.gainFactor[boardSettings.gain_v], 6);
  }
  tft.println("Short JP1.");
  waitForUser();

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
    { 0, 1, 560.0, 650.0, 1.56, false },
    { 0, 2, 2690.0, 3100.0, 1.56, false },
    { 1, 0, 990.0, 1100.0, 1.56, true },
    { 2, 0, 9900.0, 11000.0, 1.56, true },
    { 0, 3, 9900.0, 11000.0, 1.4, false },
    { 3, 0, 99000.0, 110000.0, 1.56, true }
  };
  boardSetPGAGainV(PGA_GAIN_1);

  float current, calR;
  for (uint preset = 0; preset < 7; preset++)
  {
    boardSetLCRRange(calSetups[preset].range);
    boardSetPGAGainI(calSetups[preset].gain);
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
      //waitForUser();
      tft.println("Enter exact value:");
      while (!enterFloat(&calR, calSetups[preset].calRmin, calSetups[preset].calRmax)) {
      }
      tft.fillScreen(ILI9341_BLACK);
      tft.setFont();
      tft.setTextSize(2);
      tft.setTextColor(ILI9341_WHITE);
      tft.setCursor(0, 0);
      
      tft.println("measure...");
      tft.updateScreen();
    }
    if (calSetups[preset].calRange)
    {
      calInB.transmissionFactor[boardSettings.range] = 1.0;
    }
    else
    {
      calInB.gainFactor[boardSettings.gain_i] = 1.0;
    }
    updateReadings();
    updateReadings();
    current = adReadings.v_rms / calR;
    tft.print("I rms= ");
    tft.println(current, 6);
    tft.print("h V= ");
    tft.print(adHeadroom(adReadings.v_peak));
    tft.print("dB I= ");
    tft.print(adHeadroom(adReadings.i_peak));
    tft.println("dB");
    if (calSetups[preset].calRange)
    {
      calInB.transmissionFactor[boardSettings.range] = current / adReadings.i_rms;
      tft.println(calInB.transmissionFactor[boardSettings.range], 6);
    }
    else
    {
      calInB.gainFactor[boardSettings.gain_i] = current / adReadings.i_rms;
      tft.println(calInB.gainFactor[boardSettings.gain_i], 6);
    }
    waitForUser();
  }

  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  
  tft.println("Remove JP1.");
  waitForUser();
  
  // write calibration data
  calSaveData();
  
  tft.println("Please restart device.");
  while(1)
  {}
}
