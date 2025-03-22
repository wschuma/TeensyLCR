#ifndef UTILS_H_
#define UTILS_H_

#include <Arduino.h>
#include "src/utils/btn_bar_menu.h"

enum menu_item: int
{
  APP_DEFAULT = 0,
  SELECT_FUNCTION,
  LCR_SET_FREQUENCY = 100,
  LCR_SET_LEVEL = 200,
};

extern int activeMenu;

extern BtnBarMenu appSelectMenu;

void initAppSelectMenu();
void runActiveApplication(uint activeFunction);



#endif