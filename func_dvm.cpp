#include "func_dvm.h"
#include "globals.h"
#include "settings.h"
#include "board.h"
#include "audio_design.h"
#include "displayhelp.h"
#include "src/utils/btn_bar_menu.h"
#include "apps.h"


void dvmDisplay()
{
  disp_val_t val;
  
  tft.fillRect(0, 0, 320, tft.height() - 69, ILI9341_BLACK);
  
  getDisplValue(adReadings.v_rms, 6, -4, &val);
  val.unit = "V";
  drawPrimaryDisplay(&val);
  
  tft.updateScreen();
}

void drawMenu()
{
  appSelectMenu.draw();
  tft.updateScreen();
}

void dvmCleanup()
{
  tft.updateChangedAreasOnly(false);
}

void functionDvm()
{
  // setup
  static const uint DISPLAY_UPDATE_RATE_MIN = 200;  // ms
  
  board.setPGAGainV(PGA_GAIN_1);
  //setOutputAmplitude(0);
  adSetOutputOffset(0);
  dvmDisplay();
  tft.updateChangedAreasOnly(true);
  
  static elapsedMillis displayUpdate = 0;
  activeMenu = SELECT_FUNCTION;

  TS_Point p;
  // loop
  while (1)
  {
    adAverageReadings();
    if (displayUpdate >= DISPLAY_UPDATE_RATE_MIN && adDataAvailable)
    {
      adDataAvailable = false;
      displayUpdate = 0;
      dvmDisplay();
    }
    char key = keypad.getKey();
    if (key)
    {
      switch (key) {
        case '2':
          activeMenu = SELECT_FUNCTION;
          drawMenu();
          break;
        case '1':
          activeMenu = APP_DEFAULT;
          drawMenu();
          break;
        default:
          break;
      }
    }
    if (!getTouchPoint(&p))
      continue;
      
    if (activeMenu == APP_DEFAULT) {
    }
    else if (activeMenu == SELECT_FUNCTION) {
      key = appSelectMenu.processTSPoint(p);
      if (key != BTN_BAR_MENU_EVENT_IDLE) {
        appId = key;
        dvmCleanup();
        return;
      }
      else
        tft.updateScreen();
    }
  }
}
