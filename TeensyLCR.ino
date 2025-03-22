#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include "audio_design.h"
#include "apps.h"
#include "board.h"
#include "calibration.h"
#include "globals.h"
#include "helper.h"
#include "settings.h"

#if AUDIO_BLOCK_SAMPLES != 192
#error Wrong block size! Edit AudioStream.h.
#endif

void setup() {
  Serial.begin(9600);
  board.init();
  
  tft.useFrameBuffer(0);
  tft.fillScreen(ILI9341_BLACK);
  tft.setRotation(1); 
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);

  tft.setTextSize(3);
  tft.println("TeensyLCR");
  
  tft.setTextSize(2);
  tft.print("Teesyduino ");
  tft.println(TEENSYDUINO);
  tft.println("run selftest");
  bool ok = board.selftest();
  if (!ok) {
    String msg = "selftest failed. system halted.";
    tft.println(msg);
    Serial.println(msg);
    while(1);
  }

  delay(100);

  tft.println("init settings");
  initSettings();
  calInit();
  loadSettings();

  tft.println("init audio");
  adInit();

  tft.println("init USB");
  myusb.begin();
  
  tft.println("init menu");
  initAppSelectMenu();

#ifdef DBG_VERBOSE
  tft.println("start application");
#endif

  delay(500);
  tft.useFrameBuffer(1);
  tft.updateChangedAreasOnly(true);

  // start calibration if key '1' is pressed and hold during startup
  if (keypad.getKeys())
    if (keypad.isPressed('1'))
      functionCalib();
}

void loop() {
  runActiveApplication(appId);
  //saveFunction();
}
