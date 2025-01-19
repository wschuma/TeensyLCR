#include "audio_design.h"
#include "autorange.h"
#include "board.h"
#include "settings.h"


/*
 * Calc headroom expansion to compensate for codec attenuation > 40kHz.
 */
float getHeadroomExp()
{
  float hdr = 0;
  float f = adGetFrequency();
  if (f > 40000.0)
  {
    hdr = (f - 40000.0) * 1.6e-4;
  }
  return hdr;
}

/*
 * Set range for voltage and current gain automatically.
 * Returns true if range was changed.
 */
bool autoRangeVI()
{
  static const float LCR_MINIMUM_HEADROOM_V = 1.0; // dB
  static const float LCR_MINIMUM_HEADROOM_I = 1.0; // dB
  static const float LCR_HEADROOM_RANGE_V = 16.0;  // dB
  static const float LCR_HEADROOM_RANGE_I = 16.0;  // dB

  bool active = false;
  float v_headroom = adHeadroom(adReadings.v_peak);
  float i_headroom = adHeadroom(adReadings.i_peak);
  float hdrExp = getHeadroomExp();
  
  // check headroom
  if (v_headroom < (LCR_MINIMUM_HEADROOM_V + hdrExp) && boardSettings.gain_v > 0)
  {
    // reduce voltage gain
    boardSetPGAGainV(--boardSettings.gain_v);
    active = true;
  }
  else if (v_headroom > (LCR_HEADROOM_RANGE_V + hdrExp) && boardSettings.gain_v < 3)
  {
    // increase voltage gain
    boardSetPGAGainV(++boardSettings.gain_v);
    active = true;
  }
  if (i_headroom < (LCR_MINIMUM_HEADROOM_I + hdrExp) && boardSettings.gain_i > 0)
  {
    // reduce current gain
    boardSetPGAGainI(--boardSettings.gain_i);
    active = true;
  }
  else if (i_headroom > (LCR_HEADROOM_RANGE_I + hdrExp) && boardSettings.gain_i < 3)
  {
    // increase current gain
    boardSetPGAGainI(++boardSettings.gain_i);
    active = true;
  }

  return active;
}

/*
 * Set range for LCR impedance automatically.
 * Returns true if range was changed.
 */
bool autoRangeZ()
{
  float z = adReadings.v_rms / adReadings.i_rms;
  float f = adGetFrequency();
  
  // check range with hysteresis
  if (z < 450)
  {
    if (boardSettings.range != 0)
    {
      boardSetLCRRange(0);
      return true;
    }
    else
      return false;
  }
  else if (z < 550)
  {
    if (boardSettings.range <= 1)
      return false;
    else
    {
      boardSetLCRRange(0);
      return true;
    }
  }
  else if (z < 4500)
  {
    if (boardSettings.range != 1)
    {
      boardSetLCRRange(1);
      return true;
    }
    else
      return false;
  }
  else if (z < 5500)
  {
    if(boardSettings.range == 1 || boardSettings.range == 2)
      return false;
    else
    {
      boardSetLCRRange(1);
      return true;
    }
  }
  else if (z < 45000)
  {
    if (boardSettings.range != 2)
    {
      boardSetLCRRange(2);
      return true;
    }
    else
      return false;
  }
  else if (z < 55000)
  {
    if (boardSettings.range >= 2)
      return false;
    else
    {
      boardSetLCRRange(2);
      return true;
    }
  }
  else
    if (f > 11000)
    {
      if (boardSettings.range != 2)
      {
        boardSetLCRRange(2);
        return true;
      }
    }
    else
    {
      if (boardSettings.range != 3)
      {
        boardSetLCRRange(3);
        return true;
      }
    }
  return false;
}

/*
 * Set ranges for voltage and current gain and measured impedance automatically.
 */
RangingState autoRange(bool holdRange, bool force)
{
  static bool rangingActive = false;
  
  bool ranging = autoRangeVI();
  
  if (!holdRange) {
    ranging |= autoRangeZ();
  }

  if (force || (!rangingActive && ranging))
  {
    // started ranging
    rangingActive = true;
    adSetAveraging(32);
    return RangingState::Started;
  }
  if (rangingActive && ranging)
  {
    // ranging is active, readings are not valid yet.
    adResetReadings();
    return RangingState::Active;
  }
  if (rangingActive && !ranging)
  {
    // finished ranging
    rangingActive = false;
    return RangingState::Finished;
  }
  
  return RangingState::None;
}
