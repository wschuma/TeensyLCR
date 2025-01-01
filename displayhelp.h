#ifndef displayhelp_h
#define displayhelp_h

#include <Arduino.h>
#include <TimeLib.h>
#include "board.h"
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


uint get_list_entry(const char** list, uint length, uint selected);
bool enterNr(uint* val, int min, int max);
float enterFloat(uint maxDigits, bool volt);
bool enterFloat(float* val, float min, float max);
bool enterFrequency(float* f, float max);
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

#endif