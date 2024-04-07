#include "sysmenu.h"
#include "src/utils/btn_bar_menu.h"
#include "src/utils/usbstorage.h"
#include "board.h"
#include "globals.h"
#include "displayhelp.h"
#include <TimeLib.h>

BtnBarMenu sysMenu(&tft);
int sysMenuBtnExit;

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  tft.print(":");
  if(digits < 10)
    tft.print('0');
  tft.print(digits);
}

void digitalClockDisplay() {
  // digital clock display of the time
  tft.print(year()); 
  tft.print("-");
  tft.print(month());
  tft.print("-");
  tft.print(day());
  tft.print(" ");
  tft.print(hour());
  printDigits(minute());
  printDigits(second());
  tft.println();
}

void sysMenuUpdate()
{
  tft.fillRect(0, 0, 320, tft.height() - 69, ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.println("[ SYSTEM MENU ]");
  digitalClockDisplay();
  
  tft.print("Temp.: ");
  tft.print(temperature.readTemperatureC(), 1);
  tft.println(" C");

  if(usbDrive.msDriveInfo.connected) 
    tft.println(F("USB:   connected"));
  else
    tft.println(F("USB:   not connected"));

  tft.updateScreen();
}

void sysMenuShow()
{
  // setup
  char key;
  static elapsedMillis displayUpdate = 1000;
  
  tft.fillScreen(ILI9341_BLACK);

  sysMenu.init(btn_feedback, "System Menu");
  sysMenuBtnExit = sysMenu.add("Close");
  sysMenu.draw();

  tft.setFont();
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_GREEN);

  TS_Point p;
  // loop
  while(1)
  {
    myusb.Task();

    if (getTouchPoint(&p))
    {
      key = sysMenu.processTSPoint(p);
      if (key == sysMenuBtnExit)
        return;
    }

    if (displayUpdate >= 1000)
    {
      sysMenuUpdate();
      displayUpdate = 0;
    }

    delay(10); // Slow it down a bit...
  }
}