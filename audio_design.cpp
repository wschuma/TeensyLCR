#include "audio_design.h"
#include "src/audio/analyze_mean.h"
#include "src/audio/control_cs4272_192k.h"

#include "settings.h"

// GUItool: begin automatically generated code
AudioSynthWaveform        sine1;          //xy=376,266
AudioInputI2Sslave       audioInI2S1;      //xy=558,539
AudioSynthWaveform       squarewave_90;  //xy=560,618
AudioOutputI2Sslave      audioOutI2S1;      //xy=562,283
AudioSynthWaveform       squarewave;     //xy=566,459
AudioAnalyzeRMS          analyzeRmsI;           //xy=845,678
AudioAnalyzeRMS          analyzeRmsV;           //xy=845,408
AudioAnalyzeMean          analyzeMeanI;           //xy=937,678
AudioAnalyzeMean          analyzeMeanV;           //xy=941,408

AudioEffectMultiply      multiply4;      //xy=845,605
AudioEffectMultiply      multiply3;      //xy=845,565
AudioEffectMultiply      multiply1;      //xy=845,476
AudioEffectMultiply      multiply2;      //xy=845,524

AudioAnalyzeMean          mean1;           //xy=993,477
AudioAnalyzeMean          mean2; //xy=991,526
AudioAnalyzeMean          mean3; //xy=993,568
AudioAnalyzeMean          mean4; //xy=988,604

AudioAnalyzePeak         peakV; //xy=845,360
AudioAnalyzePeak         peakI; //xy=845,724
AudioConnection          patchCord2(sine1, 0, audioOutI2S1, 0);
AudioConnection          patchCord3(audioInI2S1, 0, analyzeRmsV, 0);
AudioConnection          patchCord6(audioInI2S1, 1, analyzeRmsI, 0);
AudioConnection          patchCord4(audioInI2S1, 0, multiply1, 1);
AudioConnection          patchCord5(audioInI2S1, 0, multiply2, 1);
AudioConnection          patchCord7(audioInI2S1, 1, multiply3, 1);
AudioConnection          patchCord8(audioInI2S1, 1, multiply4, 1);
AudioConnection          patchCord9(squarewave_90, 0, multiply4, 0);
AudioConnection          patchCord10(squarewave_90, 0, multiply2, 0);
AudioConnection          patchCord11(squarewave, 0, multiply1, 0);
AudioConnection          patchCord12(squarewave, 0, multiply3, 0);

AudioConnection          patchCord13(multiply1, mean1);
AudioConnection          patchCord14(multiply2, mean2);
AudioConnection          patchCord15(multiply3, mean3);
AudioConnection          patchCord16(multiply4, mean4);

AudioConnection          patchCord17(audioInI2S1, 0, peakV, 0);
AudioConnection          patchCord18(audioInI2S1, 1, peakI, 0);
AudioControlCS4272_192k  CS4272;         //xy=580,165
// GUItool: end automatically generated code

static const uint AVG_BUFFER_LENGTH = 256;
static const uint AVG_BUFFER_VALUES = 8;

typedef struct ad_raw_readings_struct_t {
  float mean1;
  float mean2;
  float mean3;
  float mean4;
  float rms_v;
  float rms_i;
  float mean_v;
  float mean_i;
} ad_raw_readings_t;

typedef union results_union_t {
  ad_raw_readings_t readings;
  float buffer[AVG_BUFFER_VALUES];
} results_pk_t;

results_pk_t avgBuffer[AVG_BUFFER_LENGTH];
uint avgBufferPos = 0;
uint avgBufferFill = 0;

uint _averaging;
ad_readings_t adReadings;
bool adDataAvailable = false;
float _frequency;

void adInit() {
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
void adSetOutputFrequency(float frequency) {
  sine1.frequency(frequency);
  squarewave.frequency(frequency);
  squarewave_90.frequency(frequency);
  adResetReadings();
  _frequency = frequency;
}

/*
 * Set board output voltage in Vp.
 */
void adSetOutputAmplitude(float amplitude) {
  float amplitude_a = amplitude * calOutA.transmissionFactor * calOutA.gainFactor;
  sine1.amplitude(amplitude_a);
  adResetReadings();
}

/*
 * Set board output offset voltage in V.
 */
void adSetOutputOffset(float offset) {
  float offset_a = offset * calOutA.transmissionFactor * calOutA.gainFactor;
  sine1.offset(offset_a);
  adResetReadings();
}

/*
 * Reset the phase angle of the square wave signals.
 */
void adResetSquarewavePhase() {
  AudioNoInterrupts();
  
  squarewave.phase(0);
  squarewave_90.phase(90);

  AudioInterrupts();
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
 * Reset moving average buffer.
 */
void adResetReadings()
{
  avgBufferFill = 0;
  adDataAvailable = false;
}

void calcMovingAvg(uint avg, float results[AVG_BUFFER_VALUES])
{
  uint pos = avgBufferPos;
  uint n = avg;
  // reset results
  for (uint idx = 0; idx < AVG_BUFFER_VALUES; idx++)
    results[idx] = 0;
  // calc sum
  while (n > 0)
  {
    pos = --pos % AVG_BUFFER_LENGTH;
    for (uint idx = 0; idx < AVG_BUFFER_VALUES; idx++)
      results[idx] += avgBuffer[pos].buffer[idx];
    n--;
  }
  for (uint idx = 0; idx < AVG_BUFFER_VALUES; idx++)
    results[idx] /= avg;
}

void adAverageReadings() {
  results_pk_t results;
  if (mean1.available() && mean2.available() && mean3.available() && mean4.available() && analyzeRmsV.available() && analyzeRmsI.available()) {
    // fill buffer
    avgBuffer[avgBufferPos].readings.mean1 = mean1.read();
    avgBuffer[avgBufferPos].readings.mean2 = mean2.read();
    avgBuffer[avgBufferPos].readings.mean3 = mean3.read();
    avgBuffer[avgBufferPos].readings.mean4 = mean4.read();
    avgBuffer[avgBufferPos].readings.rms_v = analyzeRmsV.read();
    avgBuffer[avgBufferPos].readings.rms_i = analyzeRmsI.read();
    avgBuffer[avgBufferPos].readings.mean_v = analyzeMeanV.read();
    avgBuffer[avgBufferPos].readings.mean_i = analyzeMeanI.read();
    
    avgBufferPos++;
    if (avgBufferPos == AVG_BUFFER_LENGTH)
      avgBufferPos = 0;

    if (avgBufferFill < AVG_BUFFER_LENGTH)
      avgBufferFill++;
    
    if (avgBufferFill >= _averaging) {
      uint32_t start = micros();
      calcMovingAvg(_averaging, results.buffer);
      adReadings.time = micros() - start;

      // get readings
      adReadings.v_peak = peakV.read();
      adReadings.i_peak = peakI.read();
      adReadings.v_rms = results.readings.rms_v * calInA.transmissionFactor * calInA.gainFactor[boardSettings.gain_v];
      adReadings.i_rms = results.readings.rms_i * calInB.transmissionFactor[boardSettings.range] * calInB.gainFactor[boardSettings.gain_i];
      adReadings.v_mean = results.readings.mean_v * calInA.transmissionFactor * calInA.gainFactor[boardSettings.gain_v];
      adReadings.i_mean = results.readings.mean_i * calInB.transmissionFactor[boardSettings.range] * calInB.gainFactor[boardSettings.gain_i];
      adReadings.a1 = atan2(results.readings.mean2, results.readings.mean1);
      adReadings.a2 = atan2(results.readings.mean4, results.readings.mean3);
      adReadings.phase_raw = adReadings.a1 - adReadings.a2;
      adReadings.phase = adReadings.phase_raw;
      adReadings.mean1 = results.readings.mean1;
      adReadings.mean2 = results.readings.mean2;
      adReadings.mean3 = results.readings.mean3;
      adReadings.mean4 = results.readings.mean4;
      
      // cap phase to +-90°
      if (-adReadings.phase > (M_PI + M_PI_2)) {
        adReadings.phase = M_PI + M_PI - adReadings.phase;
      }
      else if (-adReadings.phase > M_PI_2) {
        adReadings.phase = -M_PI - adReadings.phase;
      }
      else if (adReadings.phase > (M_PI + M_PI_2)) {
        adReadings.phase = -M_PI - M_PI + adReadings.phase;
      }
      else if (adReadings.phase > M_PI_2) {
        adReadings.phase = M_PI - adReadings.phase;
      }

      adDataAvailable = true;
    }
  }
}

/*
 * Returns the currently used output frequency.
 */
float adGetFrequency() {
  return _frequency;
}

/*
 * Returns the currently used averaging value.
 */
uint adGetAveraging()
{
  return _averaging;
}

/*
 * Set averaging value for audio readings.
 */
void adSetAveraging(uint avg)
{
  _averaging = avg;
  adResetReadings();
}
