# TeensyLCR

 - A Teensy 4.1 based LCR meter
 - measure capacitance, inductance, resistance and other complex parameters
 - 4 wire measurement
 - using an audio codec to measure voltage and current
 - ILI9341 320x240 Touchscreen with user interface

# Features

## LCR Meter

 - Basic accuracy of at least 1%
 - There are four ranges: 100R, 1k, 10k, 100k. The range will be selected automatically according to the impedance of the DUT.
 - Selectable test frequencies: 100 Hz to 90 kHz in 100 Hz steps.
 - Selectable test levels: 300 mV, 600 mV, 1 V

The following complex parameter of the DUT can be measured (fixed combinations of them):
 - Rs   // equivalent series resistance (ESR)
 - Rp   // parallel resistance
 - Cs   // series capacitance
 - Cp   // parallel capacitance
 - Ls   // series inductance
 - Lp   // parallel inductance
 - Phi  // phase angle of impedance
 - Xs   // series reactance
 - Z    // impedance
 - Q    // quality factor
 - D    // dissipation factor
 - G    // conductance
 - B    // susceptance

Moving average of the readings can be set from 1 to 256.

Save screenshots to USB thump drive:

![screenshot](docs/screenshot_2024-04-12_20-04-12.bmp)

## Frequency Generator

 - A simple frequency generator for debug purpose
 - Signal output at HCUR connector
 - Waveforms: sinus only
 - Frequency: 1Hz - 90kHz
 - Amplitude: 1mVp - 2.4Vp
 - Offset: 0 - 1V
 - about 100R output impedance

## Volt Meter

 - A simple voltmeter for debug purpose
 - use HPOT and LPOT inputs

# Hardware

![Board](docs/Board%20Top%20View.png)

 - [Schematic](hardware/Schematic_TeensyLCR_R1_2024-04-08.pdf)
 - analog frontend using an auto balancing impedance bridge
 - 4 impedance ranges
 - PGA for voltage and current signals
 - Touchscreen, keypad and rotary encoder
 - temperature sensor near codec to measure board temperature
 - I2C eeprom to store calibration data and other settings

# Software implementation details

 - Teensy Audio design:

![Teensy Audio design](docs/Teensy_AudioSystemDesign.PNG)

 - Phase calculation works exactly as described in [TIDA-060029](https://www.ti.com/tool/TIDA-060029) LCR meter analog front end reference design.
