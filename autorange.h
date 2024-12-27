#ifndef autorange_h
#define autorange_h

#include <Arduino.h>

enum class RangingState
{
  None,
  Started,
  Active,
  Finished,
};

RangingState autoRange(bool holdRange, bool force);

#endif
