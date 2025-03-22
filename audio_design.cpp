#include "audio_design.h"
#include "globals.h"
#include "src/audio/analyze_mean.h"
#include "src/audio/control_cs4272_192k.h"

#include "board.h"
#include "calibration.h"
#include "settings.h"

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       sine1;          //xy=388,281
AudioInputI2Sslave       audioInI2S1;    //xy=570,554
AudioSynthWaveform       squarewave_90;  //xy=572,633
AudioOutputI2Sslave      audioOutI2S1;   //xy=574,298
AudioSynthWaveform       squarewave;     //xy=578,474
AudioAnalyzePeak         peakI;          //xy=838,731
AudioAnalyzePeak         peakV;          //xy=842,385
AudioEffectMultiply      multiply1;      //xy=857,491
AudioEffectMultiply      multiply2;      //xy=857,539
AudioEffectMultiply      multiply3;      //xy=857,580
AudioEffectMultiply      multiply4;      //xy=857,620
AudioAnalyzeRMS          analyzeRmsI;    //xy=857,693
AudioAnalyzeMean         analyzeMeanI; //xy=860,769
AudioAnalyzeRMS          analyzeRmsV;    //xy=863,423
AudioAnalyzeMean         analyzeMeanV; //xy=867,345
AudioAnalyzeMean         mean3; //xy=994.4999656677246,580.1666822433472
AudioAnalyzeMean         mean1; //xy=995,491
AudioAnalyzeMean         mean2;  //xy=995,539
AudioAnalyzeMean         mean4; //xy=996,620
AudioConnection          patchCord1(sine1, 0, audioOutI2S1, 0);
AudioConnection          patchCord2(audioInI2S1, 0, analyzeRmsV, 0);
AudioConnection          patchCord3(audioInI2S1, 0, multiply1, 1);
AudioConnection          patchCord4(audioInI2S1, 0, multiply2, 1);
AudioConnection          patchCord5(audioInI2S1, 0, peakV, 0);
AudioConnection          patchCord6(audioInI2S1, 0, analyzeMeanV, 0);
AudioConnection          patchCord7(audioInI2S1, 1, analyzeRmsI, 0);
AudioConnection          patchCord8(audioInI2S1, 1, multiply3, 1);
AudioConnection          patchCord9(audioInI2S1, 1, multiply4, 1);
AudioConnection          patchCord10(audioInI2S1, 1, peakI, 0);
AudioConnection          patchCord11(squarewave_90, 0, multiply4, 0);
AudioConnection          patchCord12(squarewave_90, 0, multiply2, 0);
AudioConnection          patchCord13(audioInI2S1, 1, analyzeMeanI, 0);
AudioConnection          patchCord14(squarewave, 0, multiply1, 0);
AudioConnection          patchCord15(squarewave, 0, multiply3, 0);
AudioConnection          patchCord16(multiply1, mean1);
AudioConnection          patchCord17(multiply2, mean2);
AudioConnection          patchCord18(multiply3, mean3);
AudioConnection          patchCord19(multiply4, mean4);
AudioControlCS4272       CS4272;         //xy=592,180
// GUItool: end automatically generated code


uint adMinAveraging = 1;
ad_readings_t adReadings;
bool adDataAvailable = false;
float _frequency;
uint32_t adBlocksToAnalyze = 100;
bool adDiscardResults = false;


/*
 * Reset the phase angle of the square wave signals.
 */
void adResetSquarewavePhase()
{
  AudioNoInterrupts();
  
  squarewave.phase(0);
  squarewave_90.phase(90);

  AudioInterrupts();
}

void adInit()
{
  AudioMemory(34);
  CS4272.enable();

  adSetOutputFrequency(AUDIO_SAMPLE_RATE_EXACT / 48);
  sine1.begin(WAVEFORM_SINE);
  
  // init squarewave signals
  squarewave.amplitude(1);
  squarewave_90.amplitude(1);
  squarewave.begin(WAVEFORM_SQUARE);
  squarewave_90.begin(WAVEFORM_SQUARE);
  adResetSquarewavePhase(); 
}

/*
 * Set frequency of output signal in Hz.
 */
void adSetOutputFrequency(float frequency)
{
  sine1.frequency(frequency);
  squarewave.frequency(frequency);
  squarewave_90.frequency(frequency);
  adResetSquarewavePhase();
  _frequency = frequency;
  adSetMinAveraging(adMinAveraging);
}

/*
 * Set board output voltage in Vp.
 */
void adSetOutputAmplitude(float amplitude)
{
  float amplitude_a = amplitude * calOutA.transmissionFactor * calOutA.gainFactor;
  sine1.amplitude(amplitude_a);
  adResetReadings();
}

/*
 * Set board output offset voltage in V.
 */
void adSetOutputOffset(float offset)
{
  float offset_a = offset * calOutA.transmissionFactor * calOutA.gainFactor;
  sine1.offset(offset_a);
  adResetReadings();
}

/*
 * Calculate digital headroom in dBFS.
 */
float adHeadroom(float peaklevel)
{
  if (peaklevel < 3.05e-5)
    peaklevel = 3.05e-5;
  return -20 * log(peaklevel) / log(10);
}

/*
 * Set a flag to reset all analyze objects after next available block.
 */
void adResetReadings()
{
  adDiscardResults = true;
  adDataAvailable = false;
}

/*
 * Must be called frequently in the main loop.
 */
void adAverageReadings()
{
  uint32_t analyzedBlocks = mean1.available();

  if (analyzedBlocks > adBlocksToAnalyze)
  {
    // Collected more data than expected. Discard all.
    adDiscardResults = true;
  }

  if (adDiscardResults)
  {
    // All collected data needs to be discarded.
    peakV.read();
    peakI.read();
    analyzeRmsV.read();
    analyzeRmsI.read();
    analyzeMeanV.read();
    analyzeMeanI.read();
    mean1.read();
    mean2.read();
    mean3.read();
    mean4.read();
    adDiscardResults = false;
    adDataAvailable = false;
    return;
  }

  if (analyzedBlocks < adBlocksToAnalyze)
  {
    // Not enough data available yet. Collect some more to average readings.
    return;
  }

  // Enough analyzed data is avaliable. Get readings...
  adReadings.v_peak = peakV.read();
  adReadings.i_peak = peakI.read();
  adReadings.v_rms = analyzeRmsV.read() * calInA.transmissionFactor * calInA.gainFactor[board.getPGAGainV()];
  adReadings.i_rms = analyzeRmsI.read() * calInB.transmissionFactor[board.getLCRRange()] * calInB.gainFactor[board.getPGAGainI()];
  adReadings.v_mean = analyzeMeanV.read() * calInA.transmissionFactor * calInA.gainFactor[board.getPGAGainV()];
  adReadings.i_mean = analyzeMeanI.read() * calInB.transmissionFactor[board.getLCRRange()] * calInB.gainFactor[board.getPGAGainI()];
  adReadings.mean1 = mean1.read();
  adReadings.mean2 = mean2.read();
  adReadings.mean3 = mean3.read();
  adReadings.mean4 = mean4.read();
  adReadings.a1 = atan2(adReadings.mean2, adReadings.mean1);
  adReadings.a2 = atan2(adReadings.mean4, adReadings.mean3);
  adReadings.phase = adReadings.a1 - adReadings.a2;
  adDataAvailable = true;
}

/*
 * Returns the currently used output frequency.
 */
float adGetFrequency()
{
  return _frequency;
}

/*
 * Returns the currently used minimum averaging value.
 */
uint adGetMinAveraging()
{
  return adMinAveraging;
}

/*
 * Returns the effective averaging value.
 */
uint adGetAveraging()
{
  return AUDIO_BLOCK_SAMPLES / AUDIO_SAMPLE_RATE_EXACT * _frequency * adBlocksToAnalyze;
}

/*
 * Set minimum averaging value for audio readings.
 */
void adSetMinAveraging(uint avg)
{
  adMinAveraging = avg;
  const uint minBlocksToAnalyze = AUDIO_SAMPLE_RATE_EXACT / AUDIO_BLOCK_SAMPLES / LCR_MIN_FREQUENCY;
  float incAvg = AUDIO_BLOCK_SAMPLES / AUDIO_SAMPLE_RATE_EXACT * _frequency * minBlocksToAnalyze;
  adBlocksToAnalyze = minBlocksToAnalyze * ceil(adMinAveraging / incAvg);
  adResetReadings();
}
