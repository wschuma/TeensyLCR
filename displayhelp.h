#ifndef displayhelp_h
#define displayhelp_h

#include <Arduino.h>
#include <TimeLib.h>
#include "XPT2046_Touchscreen.h"

static const uint MAIN_DISPLAY_X = 31;
static const uint MAIN_DISPLAY_Y_PRIMARY = 3;
static const uint MAIN_DISPLAY_W = 280;
static const uint MAIN_DISPLAY_H = 52;
static const uint MAIN_DISPLAY_Y_SECONDARY = 65;

typedef struct disp_val_struct {
  float value;
  bool minus;
  int digits;
  char modifier;
  bool overflow;
  String str;
  String unit;
} disp_val_t;

static const uint UNIT_PREFIX_NONE = 1;
static const uint UNIT_PREFIX_MILLI = 2;
static const uint UNIT_PREFIX_KILO = 4;

uint get_list_entry(const char** list, uint length, uint selected);
bool enterNr(uint* val, int min, int max);
bool enterFloat(float* val, float min, float max, const char *unit, int unitPrefix = UNIT_PREFIX_NONE);
float enterFloat(float min, float max, const char *unit, int unitPrefix = UNIT_PREFIX_NONE);
float enterFrequency(float min, float max, const char *title);
float enterVoltage(float min, float max);
bool enterOffset(float *val, float min, float max);
bool enterTime(int* hr, int* min);
bool enterDate(int* day, int* month, int* yr);
void getDisplValue(float v, uint digits, int resolution, disp_val_t* out);
void getDisplValueExt(float v, uint digits, int resolution, disp_val_t* out, bool fixed);
bool getTouchPoint(TS_Point* p);
void drawPrimaryDisplay(disp_val_t* val);
void drawSecondaryDisplay(disp_val_t* val);
void drawPrimaryDisplay(String s);
void drawSecondaryDisplay(String s);
void printDigits(int digits);
void printDateTime(time_t time);
void showMessage(const char *msg);
void showMessage(const __FlashStringHelper *f);
void drawProgressBar(const float value);

#endif