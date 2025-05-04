#ifndef globals_h
#define globals_h


#define DBG_VERBOSE           1
//#define USE_INTERNAL_EEPROM

const uint LCR_FREQ_RESOLUTION = 10;  // frequency resolution in Hz
const uint LCR_MIN_FREQUENCY = 10;    // minimum frequency in Hz
const uint LCR_MAX_FREQUENCY = 90000; // maximum frequency in Hz

const float LCR_LEVEL_RESOLUTION = 0.001; // output level resolution in V rms
const float LCR_MIN_LEVEL = 0.03;         // minimum output level in V rms
const float LCR_MAX_LEVEL = 2.5;          // maximum output level in V rms

#endif