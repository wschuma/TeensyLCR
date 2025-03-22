#include "displayhelp.h"
#include <Arduino.h>
#include "board.h"
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
static const uint8_t INPUT_FIELD_CURSOR_Y = INPUT_FIELD_Y + 6;

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

/*
 * Draw input field and set text color and font. Set cursor to left position.
 * Doesn't update screen.
 */
void drawInputField()
{
  tft.fillRect(INPUT_FIELD_X, INPUT_FIELD_Y, INPUT_FIELD_W, INPUT_FIELD_H, ILI9341_NAVY);
  tft.drawRect(INPUT_FIELD_X, INPUT_FIELD_Y, INPUT_FIELD_W, INPUT_FIELD_H, ILI9341_WHITE);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(Arial_24);
  tft.setCursor(INPUT_FIELD_X + 4, INPUT_FIELD_CURSOR_Y);
}

/*
 * Draw input field, left justified string and cursor. Updates the screen.
 */
void drawInputField(String s)
{
  drawInputField();
  tft.print(s);
  tft.print("_");
  tft.updateScreen();
}

/*
 * Draw input field and right justified float. Updates the screen.
 */
void drawInputField(float v)
{
  drawInputField();
  String s = String(v, 1);
  int16_t x, y;
  uint16_t w, h;
  tft.getTextBounds(s, 0, 0, &x, &y, &w, &h);
  tft.setCursor(INPUT_FIELD_X + INPUT_FIELD_W - 4 - w, INPUT_FIELD_CURSOR_Y);
  tft.print(s);
  tft.updateScreen();
}

/*
 * Draw input field and time. Updates the screen.
 */
void drawInputFieldTime(int hr, int min, int cursor = -1)
{
  drawInputField();
  if (hr < 10)
    tft.print("0");
  tft.print(hr);
  tft.print(":");
  if (min < 10)
    tft.print("0");
  tft.print(min);
  if (cursor >= 0) {
    if (cursor == 1)
      cursor = 18;
    else if (cursor == 2)
      cursor = 45;
    else if (cursor >= 3)
      cursor = 63;
    tft.setCursor(INPUT_FIELD_X + 4 + cursor, INPUT_FIELD_CURSOR_Y);
    tft.print("_");
  }
  tft.updateScreen();
}

/*
 * Draw input field and date. Updates the screen.
 */
void drawInputFieldDate(int d, int m, int yr, int cursor = -1)
{
  drawInputField();
  if (d < 10)
    tft.print("0");
  tft.print(d);
  tft.print(".");
  if (m < 10)
    tft.print("0");
  tft.print(m);
  tft.print(".");
  tft.print(yr);
  if (cursor >= 0) {
    if (cursor == 1)
      cursor = 18;
    else if (cursor == 2)
      cursor = 45;
    else if (cursor == 3)
      cursor = 63;
    else if (cursor == 4)
      cursor = 126;
    else if (cursor >= 5)
      cursor = 144;
    tft.setCursor(INPUT_FIELD_X + 4 + cursor, INPUT_FIELD_CURSOR_Y);
    tft.print("_");
  }
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

bool enterFloat(float *val, float min, float max, const char *unit, int unitPrefix, bool showCancel)
{
  uint charsLeft = 7;
  String inp = "";
  char key = 0;
  int btn;
  float f;

  BtnBarMenu menu(&tft);
  menu.init(btn_feedback);
  int menuBtnCancel = 0, menuBtnEnterMilli = 0, menuBtnEnter = 0, menuBtnEnterKilo = 0;
  char displUnit[8];
  if (showCancel) {
    menuBtnCancel = menu.add("Cancel");
  }
  if (unitPrefix & UNIT_PREFIX_MILLI) {
    strcpy(displUnit, "m");
    strcat(displUnit, unit);
    menuBtnEnterMilli = menu.add(displUnit);
  }
  if (unitPrefix & UNIT_PREFIX_NONE) {
    menuBtnEnter = menu.add(unit);
  }
  if (unitPrefix & UNIT_PREFIX_KILO) {
    strcpy(displUnit, "k");
    strcat(displUnit, unit);
    menuBtnEnterKilo = menu.add(displUnit);
  }
  menu.draw();

  drawInputField("");
  TS_Point p;
  while(1)
  {
    updateParamLimitMsg(false);

    key = keypad.getKey();
    if (key)
    {
      if (key == '.') {
        // add period
        if (inp.indexOf('.') < 0) {
          inp += '.';
          drawInputField(inp);
        }
        continue;
      }
      if (key == 'D') {
        // delete char
        if (inp.length()) {
          if (inp.indexOf('.', inp.length() - 1) < 0)
            charsLeft++;
          inp.remove(inp.length() - 1);
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
    if (showCancel && btn == menuBtnCancel) {
      return false;
    } else if (unitPrefix & UNIT_PREFIX_NONE && btn == menuBtnEnter) {
      f = inp.toFloat();
      if (inp.length() && checkFloat(f, min, max)) {
        *val = f;
        return true;
      }
    } else if (unitPrefix & UNIT_PREFIX_MILLI && btn == menuBtnEnterMilli) {
      f = inp.toFloat() / 1000;
      if (inp.length() && checkFloat(f, min, max)) {
        *val = f;
        return true;
      }
    } else if (unitPrefix & UNIT_PREFIX_KILO && btn == menuBtnEnterKilo) {
      f = inp.toFloat() * 1000;
      if (inp.length() && checkFloat(f, min, max)) {
        *val = f;
        return true;
      }
    }
  }
}

/*
 * Show input window for entering floating point values. Supports cancellation.
 * Returns true, if value has enterd. Returns false if user pressed Cancel.
 */
bool enterFloat(float* val, float min, float max, const char *unit, int unitPrefix)
{
  return enterFloat(val, min, max, unit, unitPrefix, true);
}

/*
 * Show input window for entering floating point values.
 * Returns the entered value.
 */
float enterFloat(float min, float max, const char *unit, int unitPrefix)
{
  float val;
  enterFloat(&val, min, max, unit, unitPrefix, false);
  return val;
}

float enterFrequency(float min, float max, const char *title)
{
  tft.fillScreen(ILI9341_BLACK);
  tft.setFont(Arial_14);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(0, 0);
  tft.println(title);

  float f = 0;
  enterFloat(&f, min, max, "Hz", UNIT_PREFIX_NONE | UNIT_PREFIX_KILO);
  return f;
}

float enterVoltage(float min, float max)
{
  tft.fillScreen(ILI9341_BLACK);
  tft.setFont(Arial_14);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(0, 0);
  tft.println("Enter output level:");

  float v = 0;
  enterFloat(&v, min, max, "Vrms", UNIT_PREFIX_MILLI | UNIT_PREFIX_NONE);
  return v;
}

bool enterOffset(float *val, float min, float max)
{
  tft.fillScreen(ILI9341_BLACK);
  tft.setFont(Arial_14);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(0, 0);
  tft.println("Enter output offset:");

  return enterFloat(val, min, max, "V", UNIT_PREFIX_MILLI | UNIT_PREFIX_NONE);
}

bool enterTime(int* hr, int* min)
{
  int cursor = 0;
  uint inp;
  char key = 0;
  int btn;
  
  BtnBarMenu menu(&tft);
  menu.init(btn_feedback);
  int menuBtnOk = menu.add("OK");
  int menuBtnCancel = menu.add("Cancel");
  menu.draw();

  drawInputFieldTime(*hr, *min, cursor);
  
  TS_Point p;
  while(1)
  {
    key = keypad.getKey();

    if (key)
    {
      if ((int(key) >= 48) && (int(key) <= 57)) { 
        inp = key - 48;
        if (cursor == 0) {
          // hour
          if (inp <= 2) {
            *hr = inp * 10;
            cursor++;
            drawInputFieldTime(*hr, *min, cursor);
          }
        }
        else if (cursor == 1) {
          // hour
          if (*hr + inp <= 23) {
            *hr += inp;
            cursor++;
            drawInputFieldTime(*hr, *min, cursor);
          }
        }
        else if (cursor == 2) {
          // minute
          if (inp <= 5) {
            *min = inp * 10;
            cursor++;
            drawInputFieldTime(*hr, *min, cursor);
          }
        }
        else if (cursor == 3) {
          // minute
          if (*min + inp <= 59) {
            *min += inp;
            cursor = -1;
            drawInputFieldTime(*hr, *min, cursor);
          }
        }
      }
    }

    if (!getTouchPoint(&p))
      continue;
    btn = menu.processTSPoint(p);
    if (btn == menuBtnOk) {
      return true;
    } else if (btn == menuBtnCancel) {
      break;
    }
  }
  
  return false;
}

bool enterDate(int* day, int* month, int* yr)
{
  int cursor = 0;
  uint inp;
  char key = 0;
  int btn;
  
  BtnBarMenu menu(&tft);
  menu.init(btn_feedback);
  int menuBtnOk = menu.add("OK");
  int menuBtnCancel = menu.add("Cancel");
  menu.draw();

  drawInputFieldDate(*day, *month, *yr, cursor);
  
  TS_Point p;
  while(1)
  {
    key = keypad.getKey();

    if (key)
    {
      if ((int(key) >= 48) && (int(key) <= 57)) { 
        inp = key - 48;
        if (cursor == 0) {
          // day
          if (inp <= 3) {
            *day = inp * 10;
            cursor++;
            drawInputFieldDate(*day, *month, *yr, cursor);
          }
        }
        else if (cursor == 1) {
          // day
          if (*day + inp <= 31 && inp > 0) {
            *day += inp;
            cursor++;
            drawInputFieldDate(*day, *month, *yr, cursor);
          }
        }
        else if (cursor == 2) {
          // month
          if (inp <= 1) {
            *month = inp * 10;
            cursor++;
            drawInputFieldDate(*day, *month, *yr, cursor);
          }
        }
        else if (cursor == 3) {
          // month
          if (*month + inp <= 12 && inp > 0) {
            *month += inp;
            cursor++;
            drawInputFieldDate(*day, *month, *yr, cursor);
          }
        }
        else if (cursor == 4) {
          // year
          *yr += inp * 10;
          cursor++;
          drawInputFieldDate(*day, *month, *yr, cursor);
        }
        else if (cursor == 5) {
          // year
          if (*yr + inp <= 2099) {
            *yr += inp;
            cursor = -1;
            drawInputFieldDate(*day, *month, *yr, cursor);
          }
        }
      }
    }

    if (!getTouchPoint(&p))
      continue;
    btn = menu.processTSPoint(p);
    if (btn == menuBtnOk) {
      return true;
    } else if (btn == menuBtnCancel) {
      break;
    }
  }
  
  return false;
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

// utility function for digital clock display: prints preceding colon and leading 0
void printDigits(int digits)
{
  tft.print(":");
  if(digits < 10)
    tft.print('0');
  tft.print(digits);
}

/*
 * Print date and time to tft display (format YYYY-M-d hh:mm:ss).
 */
void printDateTime(time_t time)
{
  tft.print(year(time));
  tft.print('-');
  tft.print(month(time));
  tft.print('-');
  tft.print(day(time));
  tft.print(' ');
  tft.print(hour(time));
  printDigits(minute(time));
  printDigits(second(time));
}

/*
 * Show onscreen message and a menu with OK button.
 */
void showMessage(const char *msg)
{
  BtnBarMenu menu(&tft);
  menu.init(btn_feedback);
  int menuBtnOk = menu.add("OK");
  menu.draw();

  osdMessage.setMessage(msg);
  osdMessage.show();

  tft.updateScreen();
  
  TS_Point p;
  while(1)
  {
    if (!getTouchPoint(&p))
      continue;
    if (menu.processTSPoint(p) == menuBtnOk)
      return;
  }
}

/*
 * Show onscreen message and a menu with OK button.
 */
void showMessage(const __FlashStringHelper *f)
{
  showMessage((const char*)f);
}

/*
 * Draw a progress bar to screen.
 */
void drawProgressBar(const float value)
{
  int16_t w = min(1, max(0, value)) * 318;
  tft.fillRect(0, 171, 320, 13, ILI9341_BLACK);
  tft.fillRect(1, 172, w, 10, ILI9341_BLUE);
}
