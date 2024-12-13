#include <Arduino.h>
#include "board.h"
#include "Wire.h"
#include "globals.h"
#include "settings.h"
#include <TimeLib.h>


Encoder encoder(ENCODER_PINB, ENCODER_PINA);
Bounce encButton = Bounce(ENCODER_PINSW, 5);

// The Generic_LM75 class will provide 9-bit (±0.5°C) temperature for any
// LM75-derived sensor. More specific classes may provide better resolution.
Generic_LM75 temperature;

// display
ILI9341_t3n tft = ILI9341_t3n(TFT_CS_PIN, TFT_DC_PIN, TFT_RST_PIN);
DMAMEM uint16_t tft_frame_buffer[ILI9341_TFTWIDTH * ILI9341_TFTHEIGHT];
XPT2046_Touchscreen ts(TS_CS_PIN, TS_IRQ_PIN);
OSDMessage osdMessage(&tft);

// Keypad
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
    {'1','2','3','L'},
    {'4','5','6','S'},
    {'7','8','9','R'},
    {'.','0','D','C'}
};
byte rowPins[ROWS] = {KEYPAD_PIN1, KEYPAD_PIN2, KEYPAD_PIN3, KEYPAD_PIN4}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {KEYPAD_PIN5, KEYPAD_PIN6, KEYPAD_PIN7, KEYPAD_PIN8}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

elapsedMillis blink;

bool isI2CDeviceConnected(int address)
{
  Wire.beginTransmission(address);
  return (Wire.endTransmission() == 0);
}

bool boardSelftest() {
  bool result = true;
  Wire.begin();
  
  /*
  if (!isI2CDeviceConnected(I2C_ADDR_CODEC))
  {
    result = false;
    tft.println("ERROR: Can't find codec!");
  }
  else
  {
    tft.println("codec ok");
  }*/
  if (!isI2CDeviceConnected(I2C_ADDR_TEMPSENSOR))
  {
    result = false;
    tft.println("ERROR: Can't find temp sensor!");
  }
  else
  {
    tft.println("temp sensor ok");
  }
  if (!isI2CDeviceConnected(I2C_ADDR_EEPROM))
  {
    result = false;
    tft.println("ERROR: Can't find eeprom!");
  }
  else
  {
    tft.println("eeprom ok");
  }
  
  return result;
}

time_t getTeensyTime()
{
  return Teensy3Clock.get();
}

void boardInit() {
  // init digital pins
  pinMode(STATUS_LED_PIN, OUTPUT);
  pinMode(DISP_BACKLIGHT_PIN, OUTPUT);
  pinMode(TFT_RST_PIN, OUTPUT);
  pinMode(RANGE_SEL_A_PIN, OUTPUT);
  pinMode(RANGE_SEL_B_PIN, OUTPUT);
  pinMode(PGA_V_A_PIN, OUTPUT);
  pinMode(PGA_V_B_PIN, OUTPUT);
  pinMode(PGA_I_A_PIN, OUTPUT);
  pinMode(PGA_I_B_PIN, OUTPUT);
  
  pinMode(ENCODER_PINSW, INPUT);

  // init status led state
  digitalWrite(STATUS_LED_PIN, HIGH);

  // set display backlight on
  digitalWrite(DISP_BACKLIGHT_PIN, HIGH);
  //digitalWrite(TFT_RST_PIN, HIGH);
  
  tft.begin();
  tft.setFrameBuffer(tft_frame_buffer);

  if (!ts.begin()) {
    Serial.println("Couldn't start touchscreen controller");
    while (1);
  }

  keypad.setDebounceTime(50);

  // set the Time library to use Teensy's RTC to keep time
  setSyncProvider(getTeensyTime);
}

void setLCRRangePins(int range) {
  digitalWrite(RANGE_SEL_A_PIN, range & 0x1);
  digitalWrite(RANGE_SEL_B_PIN, range & 0x2);
};

void setPGAGainVPins(int gain) {
  digitalWrite(PGA_V_A_PIN, gain & 0x1);
  digitalWrite(PGA_V_B_PIN, gain & 0x2);
}

void setPGAGainIPins(int gain) {
  digitalWrite(PGA_I_A_PIN, gain & 0x1);
  digitalWrite(PGA_I_B_PIN, gain & 0x2);
}

void boardSetLCRRange() {
  setLCRRangePins(range_presets[boardSettings.range]);
};

void boardSetLCRRange(uint rangePreset) {
  boardSettings.range = rangePreset;
  setLCRRangePins(range_presets[boardSettings.range]);
};

void boardSetPGAGainV() {
  setPGAGainVPins(gain_v_presets[boardSettings.gain_v]);
};

void boardSetPGAGainV(uint gainPreset) {
  boardSettings.gain_v = gainPreset;
  setPGAGainVPins(gain_v_presets[boardSettings.gain_v]);
};

void boardSetPGAGainI() {
  setPGAGainIPins(gain_i_presets[boardSettings.gain_i]);
};

void boardSetPGAGainI(uint gainPreset) {
  boardSettings.gain_i = gainPreset;
  setPGAGainIPins(gain_i_presets[boardSettings.gain_i]);
};

void btn_feedback()
{
  digitalWrite(STATUS_LED_PIN, HIGH);
  delay(50);
  digitalWrite(STATUS_LED_PIN, LOW);
}

void boardBlinkStatusLed()
{
  if (blink >= 1000)
  {
    blink = 0;
    digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
  }
}
/*
bool encoderUpdate()
{
  long newPosition = encoder.read() >> 2;

  if (newPosition != oldPosition) {
    if (newPosition < 0) {
      newPosition = length - 1;
      encoder.write(newPosition<<2);
    }
    else if (newPosition >= length)
    {
      newPosition = newPosition - length;
      encoder.write(newPosition<<2);
    }
    oldPosition = newPosition;
    return true;
  }
  return false;
}*/