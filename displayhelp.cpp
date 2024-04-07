#include "displayhelp.h"
#include <Arduino.h>
#include <ili9341_t3n_font_Arial.h>
#include "src/utils/btn_bar_menu.h"

//****************************************************************************
// This is calibration data for the raw touch data to the screen coordinates
//****************************************************************************
#define TS_MINX 3793
#define TS_MINY 3701
#define TS_MAXX 380
#define TS_MAXY 320
//#define SCREEN_ORIENTATION_1

static const uint8_t INPUT_FIELD_X = 75;
static const uint8_t INPUT_FIELD_Y = 80;
static const uint8_t INPUT_FIELD_W = 170;
static const uint8_t INPUT_FIELD_H = 40;

static const uint8_t LIST_X = 30;
static const uint8_t LIST_Y = 30;
static const uint8_t LIST_ENTRY_X = LIST_X + 3;
static const uint8_t LIST_ENTRY_Y = LIST_Y + 3;
static const uint8_t LIST_ENTRY_W = 60;
static const uint8_t LIST_ENTRY_H = 30;
static const uint8_t LIST_ROW_COUNT = 4;
static const uint8_t LIST_COL_COUNT = 4;
static const uint8_t LIST_W = LIST_ENTRY_W * LIST_COL_COUNT + 6;
static const uint8_t LIST_H = LIST_ENTRY_H * LIST_ROW_COUNT + 6;
static const uint16_t LIST_COLOR_BG = ILI9341_NAVY;
static const uint16_t LIST_COLOR_FRAME = ILI9341_WHITE;
static const uint16_t LIST_COLOR_TEXT = ILI9341_WHITE;
static const uint16_t LIST_COLOR_TEXT_HL = ILI9341_BLACK;
static const uint16_t LIST_COLOR_ENTRY_BG = ILI9341_LIGHTGREY;

void draw_list_entry(const char** list, uint idx, bool highlight)
{
  uint row = idx / LIST_COL_COUNT;
  uint col = idx - row * LIST_ROW_COUNT;
  if (highlight) {
    tft.fillRect(LIST_ENTRY_X + col * LIST_ENTRY_W, LIST_ENTRY_Y + row * LIST_ENTRY_H, LIST_ENTRY_W, LIST_ENTRY_H - 4, LIST_COLOR_ENTRY_BG);
    tft.setTextColor(LIST_COLOR_TEXT_HL);
  } else {
    tft.fillRect(LIST_ENTRY_X + col * LIST_ENTRY_W, LIST_ENTRY_Y + row * LIST_ENTRY_H, LIST_ENTRY_W, LIST_ENTRY_H - 4, LIST_COLOR_BG);
    tft.setTextColor(LIST_COLOR_TEXT);
  }
  tft.setCursor(LIST_ENTRY_X + 2 + col * LIST_ENTRY_W, LIST_ENTRY_Y + 4 + row * LIST_ENTRY_H);
  tft.print(list[idx]);
}

void draw_list(const char** list, uint length)
{
  tft.fillRect(LIST_X, LIST_Y, LIST_W, LIST_H, LIST_COLOR_BG);
  tft.drawRect(LIST_X, LIST_Y, LIST_W, LIST_H, LIST_COLOR_FRAME);
  tft.setFont(Arial_14);
  tft.setTextColor(LIST_COLOR_TEXT);
  for (uint idx = 0; idx < length; idx++)
  {
    draw_list_entry(list, idx, false);
  }
}

uint get_list_entry(const char** list, uint length, uint selected)
{
  if (selected >= length)
    selected = 0;
  uint idx = selected;
  
  tft.fillScreen(ILI9341_BLACK);
  draw_list(list, length);

  // highlight selected entry
  draw_list_entry(list, idx, true);
  tft.updateScreen();
  encoder.write(idx<<2);

  while(1)
  {
    long newPosition = (encoder.read() + 2) >> 2;
    delay(10);
    
    if (newPosition != idx) {
      // clear old position
      draw_list_entry(list, idx, false);
      if (newPosition < 0) {
        idx = length - 1;
        newPosition = idx;
        encoder.write(idx<<2);
      }
      else if (newPosition >= length)
      {
        idx = newPosition - length;
        newPosition = idx;
        encoder.write(idx<<2);
      } else
        idx = newPosition;
      // highlight new position
      draw_list_entry(list, idx, true);
      tft.fillRect(0, 0, 200, 25, ILI9341_BLACK);
      tft.updateScreen();
    }

    if (encButton.update()) {
      if (encButton.risingEdge()) {
        break;
      }
    }
  }

  return idx;
}

void drawInputField(String s)
{
  tft.fillRect(INPUT_FIELD_X, INPUT_FIELD_Y, INPUT_FIELD_W, INPUT_FIELD_H, ILI9341_NAVY);
  tft.drawRect(INPUT_FIELD_X, INPUT_FIELD_Y, INPUT_FIELD_W, INPUT_FIELD_H, ILI9341_WHITE);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(Arial_24);
  tft.setCursor(INPUT_FIELD_X + 4, INPUT_FIELD_Y + 4);
  tft.print(s);
  tft.print("_");
  tft.updateScreen();
}

void drawInputField(float v)
{
  tft.fillRect(INPUT_FIELD_X, INPUT_FIELD_Y, INPUT_FIELD_W, INPUT_FIELD_H, ILI9341_NAVY);
  tft.drawRect(INPUT_FIELD_X, INPUT_FIELD_Y, INPUT_FIELD_W, INPUT_FIELD_H, ILI9341_WHITE);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(Arial_24);
  tft.setCursor(INPUT_FIELD_X + 4, INPUT_FIELD_Y + 4);
  String s = String(v, 1);
  int16_t x, y;
  uint16_t w, h;
  tft.getTextBounds(s, 0, 0, &x, &y, &w, &h);
  tft.setCursor(INPUT_FIELD_X + INPUT_FIELD_W - 4 - w, INPUT_FIELD_Y + 4);
  tft.print(s);
  tft.updateScreen();
}

void updateParamLimitMsg(bool draw)
{
  static const uint8_t MSG_X = 160;
  static const uint8_t MSG_Y = 135;
  static const uint16_t MSG_TIME = 3000;

  static bool set = false;
  static uint32_t timer = 0;
  if (draw)
  {
    // set message
    set = true;
    timer = millis() + MSG_TIME;
    tft.setTextColor(CL(255, 80, 80));
    tft.setFont(Arial_14);
    tft.setCursor(MSG_X, MSG_Y, true);
    tft.print("Parameter Limit!");
    tft.updateScreen();
    return;
  }

  if (set && millis() > timer) {
    // clear message
    set = false;
    tft.fillRect(0, MSG_Y - 10, 320, 20, ILI9341_BLACK);
    tft.updateScreen();
  }
}

bool enterNr(uint* val, int min, int max)
{
  uint digitsLeft = 8;
  String inp, tst;
  char key = 0;
  
  BtnBarMenu menu(&tft);
  int btn, menuBtnOk, menuBtnCancel;
  menu.init(btn_feedback);
  menuBtnOk = menu.add("OK");
  menuBtnCancel = menu.add("Cancel");
  menu.draw();

  drawInputField("");

  TS_Point p;
  while(1)
  {
    updateParamLimitMsg(false);

    key = keypad.getKey();

    if (key)
    {
      if (key == 'D' && inp.length()) {
        // delete char
        inp.remove(inp.length() - 1);
        digitsLeft++;
        drawInputField(inp);
        continue;
      }

      if ((int(key) >= 48) && (int(key) <= 57)) { 
        if (!digitsLeft)
          continue;
        tst = inp + key;
        if (tst.toInt() > max) {
          // entered number is out of range. display parameter limit.
          updateParamLimitMsg(true);
        } else {
          // entered number is within limits.
          inp = tst;
          digitsLeft--;
          drawInputField(inp);
        }
      }
    }

    if (encButton.update() && encButton.risingEdge()) {
      if (inp.toInt() < min) {
        // number is lower than minimum. display parameter limit.
        updateParamLimitMsg(true);
      } else {
        break;
      }
    }

    if (!getTouchPoint(&p))
      continue;
    btn = menu.processTSPoint(p);
    if (btn == menuBtnOk) {
      if (inp.toInt() < min) {
        // number is lower than minimum. display parameter limit.
        updateParamLimitMsg(true);
      } else {
        break;
      }
    } else if (btn == menuBtnCancel) {
      return false;
    }
  }
  
  if (inp.length())
    *val = inp.toInt();
  else
    *val = 0;
  return true;
}

bool checkFloat(float val, float min, float max)
{
  if (val < min) {
    updateParamLimitMsg(true);
    return false;
  }
  if (val > max) {
    updateParamLimitMsg(true);
    return false;
  }
  return true;
}

bool enterFloat(float* val, float min, float max)
{
  uint charsLeft = 8;
  String inp, tst;
  char key = 0;
  
  BtnBarMenu menu(&tft);
  int btn, menuBtnOk, menuBtnCancel;
  menu.init(btn_feedback);
  menuBtnOk = menu.add("OK");
  menuBtnCancel = menu.add("Cancel");
  menu.draw();

  drawInputField("");

  TS_Point p;
  while(1)
  {
    updateParamLimitMsg(false);

    key = keypad.getKey();
    if (key == 'D' && inp.length()) {
      // delete char
      inp.remove(inp.length() - 1);
      charsLeft++;
      drawInputField(inp);
    }
    else if ((int(key) >= 48) && (int(key) <= 57)) { 
      // add digit
      if (!charsLeft)
        continue;
      charsLeft--;
      inp = inp + key;
      drawInputField(inp);
    }
    else if (key == '.') {
      // add period
      if (inp.indexOf('.') < 0) {
        inp += '.';
        charsLeft--;
        drawInputField(inp);
      }
    }

    if (encButton.update() && encButton.risingEdge()) {
      if (checkFloat(inp.toFloat(), min, max)) {
        break;
      }
    }

    if (!getTouchPoint(&p))
      continue;
    btn = menu.processTSPoint(p);
    if (btn == menuBtnOk) {
      if (checkFloat(inp.toFloat(), min, max)) {
        break;
      }
    } else if (btn == menuBtnCancel) {
      return false;
    }
  }
  
  if (inp.length())
    *val = inp.toFloat();
  else
    *val = 0;
  return true;
}

float enterFloat(uint maxDigits, bool volt)
{
  uint charsLeft = maxDigits;
  String inp = "";
  char key = 0;
  uint8_t btn;
  float factor = 1;

  BtnBarMenu menu(&tft);
  int menuBtnVrms, menuBtnVp, menuBtnVpp;
  menu.init(btn_feedback);
  if (volt) {
    menuBtnVrms = menu.add("V rms");
    menuBtnVp = menu.add("V p");
    menuBtnVpp = menu.add("V pp");
  } else {
    menuBtnVp = menu.add("OK");
  }
  menu.draw();

  drawInputField("");
  TS_Point p;
  while(1)
  {
    key = keypad.getKey();
    if (key)
    {
      if (key == '.') {
        // add period
        if (inp.indexOf('.') < 0) {
          inp += '.';
          charsLeft--;
          drawInputField(inp);
        }
        continue;
      }
      if (key == 'D') {
        // delete char
        if (inp.length()) {
          inp.remove(inp.length() - 1);
          charsLeft++;
          drawInputField(inp);
        }
        continue;
      }
      if ((int(key) >= 48) && (int(key) <= 57)) { 
        // add digit
        if (!charsLeft)
          continue;
        inp += key;
        drawInputField(inp);
        charsLeft--;
      }
    }

    if (!getTouchPoint(&p))
      continue;
    btn = menu.processTSPoint(p);
    if (btn == menuBtnVp) {
      factor = 1;
      break;
    } else if (btn == menuBtnVrms) {
      factor = sqrtf(2);
      break;
    } else if (btn == menuBtnVpp) {
      factor = 0.5;
      break;
    }
  }
  
  if (inp.length())
    return inp.toFloat() * factor;
  else
    return 0;
}

bool enterFrequency(float* f, float max)
{
  float min = 0.1;
  uint charsLeft = 3;
  float val = 0.0;
  char key = 0;
  
  BtnBarMenu menu(&tft);
  int btn, menuBtnOk, menuBtnCancel;
  menu.init(btn_feedback);
  menuBtnOk = menu.add("OK");
  menuBtnCancel = menu.add("Cancel");
  menu.draw();

  drawInputField(val);

  TS_Point p;
  while(1)
  {
    updateParamLimitMsg(false);

    key = keypad.getKey();
    if (key == 'D' && val > 0.1) {
      // delete char
      val = int(val) / 10.0;
      charsLeft++;
      drawInputField(val);
    }
    else if ((int(key) >= 48) && (int(key) <= 57)) { 
      // add digit
      if (!charsLeft)
        continue;
      charsLeft--;
      val = val * 10 + float(key - 48) / 10.0;
      drawInputField(val);
    }

    if (encButton.update() && encButton.risingEdge()) {
      if (checkFloat(val * 1000, min, max)) {
        break;
      }
    }

    if (!getTouchPoint(&p))
      continue;
    btn = menu.processTSPoint(p);
    if (btn == menuBtnOk) {
      if (checkFloat(val * 1000, min, max)) {
        break;
      }
    } else if (btn == menuBtnCancel) {
      return false;
    }
  }
  
  *f = val * 1000;
  return true;
}

void getDisplValueExt(float v, uint digits, int resolution, disp_val_t* out, bool fixed)
{
  int exp = 0;
  char mod = ' ';
  out->overflow = false;
  
  float absVal = abs(v);
  while (absVal > 1.1e3 && !fixed)
  {
    absVal /= 1.0e3;
    exp += 3;
    if (exp > 9)
      break;
  }
  while (absVal < 0.9 && (exp-3) > resolution && !fixed)
  {
    absVal *= 1.0e3;
    exp -= 3;
    if (exp < -12)
    {
      absVal = 0;
      break;
    }
  }
  
  switch (exp) {
    case -12:
      mod = 'p';
      break;
    case -9:
      mod = 'n';
      break;
    case -6:
      mod = 'u';
      break;
    case -3:
      mod = 'm';
      break;
    case 0:
      mod = ' ';
      break;
    case 3:
      mod = 'k';
      break;
    case 6:
      mod = 'M';
      break;
    case 9:
      mod = 'G';
      break;
    default:
      out->overflow = true;
      break;
  }

  out->value = absVal;
  out->minus = (v < 0);
  out->modifier = mod;
  if (isnan(v))
    out->overflow = true;

  String s = String((int)absVal);    
  uint digitsBeforePeriod = s.length();
  if (digitsBeforePeriod > digits)
    out->overflow = true;

  if (out->overflow) {
    out->str = " OVL ";
    return;
  }

  uint digitsAfterPeriod = exp - resolution;
  if (digits - digitsBeforePeriod < digitsAfterPeriod)
  {
    digitsAfterPeriod = digits - digitsBeforePeriod;
  }
  
  // Round correctly
  double rounding = 0.5;
  for (uint8_t i = 0; i < digitsAfterPeriod; ++i)
  {
    rounding *= 0.1;
  }
  rounding += absVal;
  
  // update digitsBeforePeriod after rounding
  s = String((int)rounding);
  digitsBeforePeriod = s.length();

  // check digitsBeforePeriod again
  if (digitsBeforePeriod + digitsAfterPeriod > digits) {
    if (digitsAfterPeriod == 0) {
      out->overflow = true;
      out->str = " OVL ";
      return;
    }
    digitsAfterPeriod = digits - digitsBeforePeriod;
  }

  // Left-pads the number with zeroes
  s = "";
  for (uint n = digitsBeforePeriod + digitsAfterPeriod; n < digits; n++)
  {
    s += "0";
  }
  s += String(absVal, digitsAfterPeriod);

  out->str = s;
}

void getDisplValue(float v, uint digits, int resolution, disp_val_t* out)
{
  getDisplValueExt(v, digits, resolution, out, false);
}

bool getTouchPoint(TS_Point* p)
{
  static const int DEBOUNCE_INTERVAL = 200;
  static const int8_t TS_IDLE = 0;
  static const int8_t TS_PRESSED = 1;
  static const int8_t TS_HOLD = 2;
  static int8_t state = TS_IDLE;
  static long lastChangeMillis = 0;
  long nowMillis = millis();
  
  // If insufficient time has passed, just return none pressed
  if (lastChangeMillis + DEBOUNCE_INTERVAL > nowMillis)
    return false;
  
  // See if there's any  touch data for us
  if (!ts.tirqTouched()) {
    return false;
  }
  // You can also wait for a touch
  if (!ts.touched()) {
    if (state == TS_PRESSED || state == TS_HOLD)
    {
      state = TS_IDLE;
    }
    return false;
  }

  if (state == TS_IDLE)
  {
    state = TS_PRESSED;
  }
  else if (state == TS_PRESSED)
  {
    state = TS_HOLD;
    return false;
  }
  else if (state == TS_HOLD)
  {
    return false;
  }
  
  // Retrieve a point
  *p = ts.getPoint();

  // p is in ILI9341_t3 setOrientation 1 settings. so we need to map x and y differently.

  // Scale from ~0->4000 to tft.width using the calibration #'s
  p->x = map(p->x, TS_MINX, TS_MAXX, 0, tft.width() - 1);
  p->y = map(p->y, TS_MINY, TS_MAXY, 0, tft.height() - 1);

  // Note the time the button was pressed
  lastChangeMillis = nowMillis;  

  return true;
}

void drawPrimaryDisplay(disp_val_t* val)
{
  tft.fillRect(MAIN_DISPLAY_X, MAIN_DISPLAY_Y_PRIMARY, MAIN_DISPLAY_W, MAIN_DISPLAY_H, ILI9341_BLACK);
  tft.setFont(Arial_40);
  tft.setCursor(MAIN_DISPLAY_X, MAIN_DISPLAY_Y_PRIMARY + 1);
  // draw sign
  if (val->minus)
    tft.setTextColor(ILI9341_YELLOW);
  else
    tft.setTextColor(ILI9341_BLACK);
  tft.print("-");
  // draw value
  tft.setTextColor(ILI9341_YELLOW);
  tft.print(val->str);
  // draw unit
  tft.print(" ");
  tft.setFont(Arial_14);
  tft.print(val->modifier + val->unit);
}

void drawPrimaryDisplay(String s)
{
  tft.fillRect(MAIN_DISPLAY_X, MAIN_DISPLAY_Y_PRIMARY, MAIN_DISPLAY_W, MAIN_DISPLAY_H, ILI9341_BLACK);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setFont(Arial_40);
  tft.setCursor(MAIN_DISPLAY_X, MAIN_DISPLAY_Y_PRIMARY + 1);
  tft.print(s);
}

void drawSecondaryDisplay(disp_val_t* val)
{
  tft.fillRect(MAIN_DISPLAY_X, MAIN_DISPLAY_Y_SECONDARY, MAIN_DISPLAY_W, MAIN_DISPLAY_H, ILI9341_BLACK);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setFont(Arial_40);
  tft.setCursor(MAIN_DISPLAY_X, MAIN_DISPLAY_Y_SECONDARY + 1);
  // draw sign
  if (val->minus)
    tft.setTextColor(ILI9341_YELLOW);
  else
    tft.setTextColor(ILI9341_BLACK);
  tft.print("-");
  // draw value
  tft.setTextColor(ILI9341_YELLOW);
  tft.print(val->str);
  // draw unit
  tft.print(" ");
  tft.setFont(Arial_14);
  tft.print(val->modifier + val->unit);
}

void drawSecondaryDisplay(String s)
{
  tft.fillRect(MAIN_DISPLAY_X, MAIN_DISPLAY_Y_SECONDARY, MAIN_DISPLAY_W, MAIN_DISPLAY_H, ILI9341_BLACK);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setFont(Arial_40);
  tft.setCursor(MAIN_DISPLAY_X, MAIN_DISPLAY_Y_SECONDARY + 1);
  tft.print(s);
}
