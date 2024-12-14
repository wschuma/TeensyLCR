#include <ili9341_t3n_font_Arial.h>
#include <TimeLib.h>
#include "board.h"
#include "displayhelp.h"
#include "globals.h"
#include "helper.h"
#include "src/utils/btn_bar_menu.h"
#include "sysmenu.h"

BtnBarMenu sysMenu(&tft);

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  tft.print(":");
  if(digits < 10)
    tft.print('0');
  tft.print(digits);
}

void digitalClockDisplay() {
  // digital clock display of the time
  tft.print("Date:");
  tft.setCursor(120, tft.getCursorY());
  tft.print(year()); 
  tft.print("-");
  tft.print(month());
  tft.print("-");
  tft.println(day());
  
  tft.print("Time:");
  tft.setCursor(120, tft.getCursorY());
  tft.print(hour());
  printDigits(minute());
  printDigits(second());
  tft.println();
}

/*
 * Draw system menu to screen.
 */
void sysMenuUpdate()
{
  tft.fillRect(0, 0, 320, tft.height() - 69, ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.setFont(Arial_14);
  tft.setTextColor(ILI9341_WHITE);
  tft.println("[ SYSTEM MENU ]");
  digitalClockDisplay();
  
  tft.print("Temp.:");
  tft.setCursor(120, tft.getCursorY());
  tft.print(temperature.readTemperatureC(), 1);
  tft.println(" C");

  tft.print("USB:");
  tft.setCursor(120, tft.getCursorY());
  if(!usbDrive.msDriveInfo.connected)
    tft.print("not ");
  tft.println("connected");
}

/*
 * UI function to set RTC time.
 */
void sysMenuSetTime()
{
  int hr = 0, min = 0;

  tft.fillScreen(ILI9341_BLACK);
  tft.setFont(Arial_14);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(0, 0);
  tft.println("Enter time:");
  bool ok = enterTime(&hr, &min);
  if (ok) {
    setTime(hr, min, 0, day(), month(), year());
    Teensy3Clock.set(now());
  }
  tft.fillScreen(ILI9341_BLACK);
  sysMenu.draw();
}

/*
 * UI function to set RTC date.
 */
void sysMenuSetDate()
{
  int day = 1, month = 1, yr = 2000;

  tft.fillScreen(ILI9341_BLACK);
  tft.setFont(Arial_14);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(0, 0);
  tft.println("Enter date:");
  bool ok = enterDate(&day, &month, &yr);
  if (ok) {
    setTime(hour(), minute(), second(), day, month, yr);
    Teensy3Clock.set(now());
  }
  tft.fillScreen(ILI9341_BLACK);
  sysMenu.draw();
}

void sysMenuShow()
{
  // setup
  char key;
  static elapsedMillis displayUpdate = 1000;
  
  tft.fillScreen(ILI9341_BLACK);

  sysMenu.init(btn_feedback, "System Menu");
  int sysMenuBtnExit = sysMenu.add("Close");
  int sysMenuBtnSetTime = sysMenu.add("Set time");
  int sysMenuBtnSetDate = sysMenu.add("Set date");
  sysMenu.draw();

  TS_Point p;
  // loop
  while(1)
  {
    myusb.Task();

    if (keypad.getKey() == 'S')
      saveScreenshot(&tft);

    if (getTouchPoint(&p))
    {
      key = sysMenu.processTSPoint(p);
      if (key == sysMenuBtnExit)
        return;
      else if (key == sysMenuBtnSetTime)
        sysMenuSetTime();
      else if (key == sysMenuBtnSetDate)
        sysMenuSetDate();
    }

    if (displayUpdate >= 1000)
    {
      sysMenuUpdate();
      osdMessage.show();
      tft.updateScreen();
      osdMessage.clean();
      displayUpdate = 0;
    }

    delay(10); // Slow it down a bit...
  }
}