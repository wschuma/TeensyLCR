#include "func_generator.h"
#include <Arduino.h>
#include "settings.h"
#include "board.h"
#include "audio_design.h"
#include "globals.h"
#include "displayhelp.h"
#include "src/utils/btn_bar_menu.h"
#include "apps.h"
#include "sysmenu.h"


// Frequency in Hz
float frequency = 1000;
// Amplitude in Vp
float amplitude = sqrtf(2);
float offset = 0;
bool outputOn = false;

// Menu definition
const char *waves[] = {"Sine", "Square"};
const char *waveOpt;

BtnBarMenu genMenu(&tft);

void displayValues()
{
  disp_val_t val;
  String s;
  
  tft.fillRect(0, 0, 320, tft.height() - 69, ILI9341_BLACK);
  tft.setFont();
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_GREEN);
  tft.setCursor(0, 0);

  getDisplValue(frequency, 6, -3, &val);
  s = val.str + ' ' + val.modifier + "Hz";
  tft.print("Frequency: ");
  tft.println(s);

  getDisplValue(amplitude * 2, 6, -4, &val);
  s = val.str + ' ' + val.modifier + "Vpp";
  tft.print("Amplitude: ");
  tft.println(s);

  getDisplValue(offset, 6, -4, &val);
  s = val.str + ' ' + val.modifier + "V";
  tft.print("Offset: ");
  tft.println(s);

  tft.print("Output: ");
  if (outputOn)
    tft.println("ON");
  else
    tft.println("OFF");

  if (activeMenu == APP_DEFAULT)
    genMenu.draw();
  else if (activeMenu == SELECT_FUNCTION)
    appSelectMenu.draw();

  tft.updateScreen();
}

void setFrequency()
{
  static const float F_MIN = 1.0;
  static const float F_MAX = AUDIO_SAMPLE_RATE_EXACT / 2;

  float f = enterFrequency(F_MIN, F_MAX, "Enter frequency:");
  if (f > 0) {
    frequency = f;
    adSetOutputFrequency(frequency);
  }

  tft.fillScreen(ILI9341_BLACK);
  tft.setFont();
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_GREEN);
  tft.setCursor(0, 0);

  displayValues();
}

void setAmplitude()
{
  static const float L_MIN = 1.0e-3;
  static const float L_MAX = 2.4;

  float level = enterVoltage(L_MIN, L_MAX);
  if (level > 0) {
    amplitude = level;
    if (outputOn)
      adSetOutputAmplitude(amplitude);
  }

  tft.fillScreen(ILI9341_BLACK);
  tft.setFont();
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_GREEN);
  tft.setCursor(0, 0);

  displayValues();
}

void setOffset()
{
  static const float O_MIN = -1.0;
  static const float O_MAX = 1.0;

  float inp;
  if (enterOffset(&inp, O_MIN, O_MAX)) {
    offset = inp;
    if (outputOn)
      adSetOutputOffset(offset);
  }

  tft.fillScreen(ILI9341_BLACK);
  tft.setFont();
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_GREEN);
  tft.setCursor(0, 0);

  displayValues();
}

void setWaveform()
{
  //waveOpt = waves[1];
  genMenu.draw();
  tft.updateScreen();
}

void setOutput()
{
  outputOn = !outputOn;
  if (outputOn) {
    adSetOutputAmplitude(amplitude);
    adSetOutputOffset(offset);
  } else {
    adSetOutputAmplitude(0);
    adSetOutputOffset(0);
  }
  displayValues();
}

void printTemp() {
  Serial.print("Temperature = ");
  Serial.print(temperature.readTemperatureC());
  Serial.println(" C");
}

void generatorApplication()
{
  // setup
  outputOn = false;
  adSetOutputAmplitude(0);
  adSetOutputOffset(0);
  adSetOutputFrequency(frequency);
  board.setLCRRange(LCR_RANGE_100);
  waveOpt = waves[0];
  
  genMenu.init(btn_feedback, "FG Menu");
  genMenu.add("Freq.", setFrequency);
  genMenu.add("Ampl.", setAmplitude);
  genMenu.add("Offset", setOffset);
  genMenu.add("Wavef.", &waveOpt, setWaveform);
  
  displayValues();
  
  TS_Point p;
  // loop
  while (1)
  {
    char key = keypad.getKey();
    if (key)
    {
      switch (key) {
        case '1':
          activeMenu = APP_DEFAULT;
          genMenu.draw();
          tft.updateScreen();
          break;
        case '2':
          activeMenu = SELECT_FUNCTION;
          appSelectMenu.draw();
          tft.updateScreen();
          break;
        case '3':
          sysMenuShow();
          tft.fillScreen(ILI9341_BLACK);
          genMenu.draw();
          displayValues();
          break;
        case '8':
          printTemp();
          break;
        case 'D':
          // output on/off
          setOutput();
          break;
        default:
          break;
      }
    }
    
    if (!getTouchPoint(&p))
      continue;
      
    if (activeMenu == APP_DEFAULT) {
      genMenu.processTSPoint(p);
    }
    else if (activeMenu == SELECT_FUNCTION) {
      key = appSelectMenu.processTSPoint(p);
      if (key != BTN_BAR_MENU_EVENT_IDLE) {
        appId = key;
        return;
      }
      else
        tft.updateScreen();
    }
  }
}