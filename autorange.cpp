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
  if (v_headroom < (LCR_MINIMUM_HEADROOM_V + hdrExp) && board.getPGAGainV() > 0)
  {
    // reduce voltage gain
    board.reduceVGain();
    active = true;
  }
  else if (v_headroom > (LCR_HEADROOM_RANGE_V + hdrExp) && board.getPGAGainV() < 3)
  {
    // increase voltage gain
    board.increaseVGain();
    active = true;
  }
  if (i_headroom < (LCR_MINIMUM_HEADROOM_I + hdrExp) && board.getPGAGainI() > 0)
  {
    // reduce current gain
    board.reduceIGain();
    active = true;
  }
  else if (i_headroom > (LCR_HEADROOM_RANGE_I + hdrExp) && board.getPGAGainI() < 3)
  {
    // increase current gain
    board.increaseIGain();
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
    if (board.getLCRRange() != LCR_RANGE_100)
    {
      board.setLCRRange(LCR_RANGE_100);
      return true;
    }
    else
      return false;
  }
  else if (z < 550)
  {
    if (board.getLCRRange() <= LCR_RANGE_1K)
      return false;
    else
    {
      board.setLCRRange(LCR_RANGE_100);
      return true;
    }
  }
  else if (z < 4500)
  {
    if (board.getLCRRange() != LCR_RANGE_1K)
    {
      board.setLCRRange(LCR_RANGE_1K);
      return true;
    }
    else
      return false;
  }
  else if (z < 5500)
  {
    if(board.getLCRRange() == LCR_RANGE_1K || board.getLCRRange() == LCR_RANGE_10K)
      return false;
    else
    {
      board.setLCRRange(LCR_RANGE_1K);
      return true;
    }
  }
  else if (z < 45000)
  {
    if (board.getLCRRange() != LCR_RANGE_10K)
    {
      board.setLCRRange(LCR_RANGE_10K);
      return true;
    }
    else
      return false;
  }
  else if (z < 55000)
  {
    if (board.getLCRRange() >= LCR_RANGE_10K)
      return false;
    else
    {
      board.setLCRRange(LCR_RANGE_10K);
      return true;
    }
  }
  else
    if (f > 11000)
    {
      if (board.getLCRRange() != LCR_RANGE_10K)
      {
        board.setLCRRange(LCR_RANGE_10K);
        return true;
      }
    }
    else
    {
      if (board.getLCRRange() != LCR_RANGE_100K)
      {
        board.setLCRRange(LCR_RANGE_100K);
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
    adSetMinAveraging(10);
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
