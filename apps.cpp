#include "apps.h"
#include "func_lcr.h"
#include "func_generator.h"
#include "func_dvm.h"
#include "board.h"


BtnBarMenu appSelectMenu(&tft);
uint8_t menuBtnLCR, menuBtnGen, menuBtnDVM;
int activeMenu;


void initAppSelectMenu()
{
  appSelectMenu.init(btn_feedback, "Application Select Menu");
  menuBtnLCR = appSelectMenu.add("LCR");
  menuBtnGen = appSelectMenu.add("FG");
  menuBtnDVM = appSelectMenu.add("DVM");
}

void runActiveApplication(uint activeFunction)
{
  activeMenu = APP_DEFAULT;
  if (activeFunction == menuBtnLCR)
    lcrApplication();
  else if (activeFunction == menuBtnGen)
    generatorApplication();
  else if (activeFunction == menuBtnDVM)
    functionDvm();
  else
    activeFunction = menuBtnLCR;
}
