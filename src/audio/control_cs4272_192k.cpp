

#include <Arduino.h>
#include "control_cs4272_192k.h"
#include "Wire.h"

#define CS4272_ADDR 0x10

// Section 8.1 Mode Control
#define CS4272_MODE_CONTROL			(uint8_t)0x01
#define CS4272_MC_FUNC_MODE(x)			(uint8_t)(((x) & 0x03) << 6)
#define CS4272_MC_RATIO_SEL(x)			(uint8_t)(((x) & 0x03) << 4)
#define CS4272_MC_MASTER_SLAVE			(uint8_t)0x08
#define CS4272_MC_SERIAL_FORMAT(x)		(uint8_t)(((x) & 0x07) << 0)

// Section 8.2 DAC Control
#define CS4272_DAC_CONTROL			(uint8_t)0x02
#define CS4272_DAC_CTRL_AUTO_MUTE		(uint8_t)0x80
#define CS4272_DAC_CTRL_FILTER_SEL		(uint8_t)0x40
#define CS4272_DAC_CTRL_DE_EMPHASIS(x)		(uint8_t)(((x) & 0x03) << 4)
#define CS4272_DAC_CTRL_VOL_RAMP_UP		(uint8_t)0x08
#define CS4272_DAC_CTRL_VOL_RAMP_DN		(uint8_t)0x04
#define CS4272_DAC_CTRL_INV_POL(x)		(uint8_t)(((x) & 0x03) << 0)

// Section 8.3 DAC Volume and Mixing
#define CS4272_DAC_VOL				(uint8_t)0x03
#define CS4272_DAC_VOL_CH_VOL_TRACKING		(uint8_t)0x40
#define CS4272_DAC_VOL_SOFT_RAMP(x)		(uint8_t)(((x) & 0x03) << 4)
#define CS4272_DAC_VOL_ATAPI(x)			(uint8_t)(((x) & 0x0F) << 0)

// Section 8.4 DAC Channel A volume
#define CS4272_DAC_CHA_VOL			(uint8_t)0x04
#define CS4272_DAC_CHA_VOL_MUTE			(uint8_t)0x80
#define CS4272_DAC_CHA_VOL_VOLUME(x)		(uint8_t)(((x) & 0x7F) << 0)

// Section 8.5 DAC Channel B volume
#define CS4272_DAC_CHB_VOL			(uint8_t)0x05
#define CS4272_DAC_CHB_VOL_MUTE			(uint8_t)0x80
#define CS4272_DAC_CHB_VOL_VOLUME(x)		(uint8_t)(((x) & 0x7F) << 0)

// Section 8.6 ADC Control
#define CS4272_ADC_CTRL				(uint8_t)0x06
#define CS4272_ADC_CTRL_DITHER			(uint8_t)0x20
#define CS4272_ADC_CTRL_SER_FORMAT		(uint8_t)0x10
#define CS4272_ADC_CTRL_MUTE(x)			(uint8_t)(((x) & 0x03) << 2)
#define CS4272_ADC_CTRL_HPF(x)			(uint8_t)(((x) & 0x03) << 0)

// Section 8.7 Mode Control 2
#define CS4272_MODE_CTRL2			(uint8_t)0x07
#define CS4272_MODE_CTRL2_LOOP			(uint8_t)0x10
#define CS4272_MODE_CTRL2_MUTE_TRACK		(uint8_t)0x08
#define CS4272_MODE_CTRL2_CTRL_FREEZE		(uint8_t)0x04
#define CS4272_MODE_CTRL2_CTRL_PORT_EN		(uint8_t)0x02
#define CS4272_MODE_CTRL2_POWER_DOWN		(uint8_t)0x01

// Section 8.8 Chip ID
#define CS4272_CHIP_ID				(uint8_t)0x08
#define CS4272_CHIP_ID_PART(x)			(uint8_t)(((x) & 0x0F) << 4)
#define CS4272_CHIP_ID_REV(x)			(uint8_t)(((x) & 0x0F) << 0)


#define CS4272_RESET_PIN 2

bool AudioControlCS4272_192k::enable(void)
{
	Wire.begin();
	delay(5);
	initLocalRegs();

	// Setup Reset pin
	pinMode(CS4272_RESET_PIN, OUTPUT);

	// Drive pin low
	digitalWriteFast(CS4272_RESET_PIN, LOW);
	delay(1);

	// Release Reset
	digitalWriteFast(CS4272_RESET_PIN, HIGH);
	delay(2);

	// Set power down and control port enable as spec'd in the 
	// datasheet for control port mode
	write(CS4272_MODE_CTRL2, CS4272_MODE_CTRL2_POWER_DOWN
		| CS4272_MODE_CTRL2_CTRL_PORT_EN);

	// Wait for further setup
	delay(1);

	// Set ratio select for MCLK=512*LRCLK (BCLK = 64*LRCLK), and master mode, I2S
	// https://forum.pjrc.com/index.php?threads/73960/
	write(CS4272_MODE_CONTROL, CS4272_MC_FUNC_MODE(CS4272_RATIO_QUAD) | CS4272_MC_RATIO_SEL(2) | CS4272_MC_MASTER_SLAVE |
		CS4272_MC_SERIAL_FORMAT(1));
	write(CS4272_ADC_CTRL, CS4272_ADC_CTRL_SER_FORMAT); // Set ADC I2S format

	delay(10);
	
	// Release power down bit to start up codec
	// TODO: May need other bits set in this reg
	write(CS4272_MODE_CTRL2, CS4272_MODE_CTRL2_CTRL_PORT_EN);
	
	// Wait for everything to come up
	delay(10);


	return true;
}

bool AudioControlCS4272_192k::volumeInteger(unsigned int n)
{
	unsigned int val = 0x7F - (n & 0x7F);
	write(CS4272_DAC_CHA_VOL,CS4272_DAC_CHA_VOL_VOLUME(val));
	write(CS4272_DAC_CHB_VOL,CS4272_DAC_CHB_VOL_VOLUME(val));
	return true;
}

bool AudioControlCS4272_192k::volume(float left, float right)
{
	unsigned int leftInt,rightInt;

	leftInt = left*127 + 0.499;
	rightInt = right*127 + 0.499;

	unsigned int val = 0x7F - (leftInt & 0x7F);
	write(CS4272_DAC_CHA_VOL,CS4272_DAC_CHA_VOL_VOLUME(val));
	
	val = 0x7F - (rightInt & 0x7F);
	write(CS4272_DAC_CHB_VOL,CS4272_DAC_CHB_VOL_VOLUME(val));

	return true;
}

bool AudioControlCS4272_192k::dacVolume(float left, float right)
{
	return volume(left,right);
}

bool AudioControlCS4272_192k::muteOutput(void)
{
	write(CS4272_DAC_CHA_VOL, 
		regLocal[CS4272_DAC_CHA_VOL] | CS4272_DAC_CHA_VOL_MUTE);

	write(CS4272_DAC_CHB_VOL, 
		regLocal[CS4272_DAC_CHB_VOL] | CS4272_DAC_CHB_VOL_MUTE);

	return true;
}

bool AudioControlCS4272_192k::unmuteOutput(void)
{
	write(CS4272_DAC_CHA_VOL, 
		regLocal[CS4272_DAC_CHA_VOL] & ~CS4272_DAC_CHA_VOL_MUTE);

	write(CS4272_DAC_CHB_VOL, 
		regLocal[CS4272_DAC_CHB_VOL] & ~CS4272_DAC_CHB_VOL_MUTE);

	return true;
}

bool AudioControlCS4272_192k::muteInput(void)
{
	uint8_t val = regLocal[CS4272_ADC_CTRL] | CS4272_ADC_CTRL_MUTE(3);
	write(CS4272_ADC_CTRL,val);
	return true;
}

bool AudioControlCS4272_192k::unmuteInput(void)
{
	uint8_t val = regLocal[CS4272_ADC_CTRL] & ~CS4272_ADC_CTRL_MUTE(3);
	write(CS4272_ADC_CTRL,val);
	return true;
}

bool AudioControlCS4272_192k::enableDither(void)
{
	uint8_t val = regLocal[CS4272_ADC_CTRL] | CS4272_ADC_CTRL_DITHER;
	write(CS4272_ADC_CTRL,val);
	return true;
}

bool AudioControlCS4272_192k::disableDither(void)
{
	uint8_t val = regLocal[CS4272_ADC_CTRL] & ~CS4272_ADC_CTRL_DITHER;
	write(CS4272_ADC_CTRL,val);
	return true;
}


bool AudioControlCS4272_192k::write(unsigned int reg, unsigned int val)
{
	// Write local copy first
	if(reg > 7)
		return false;

	regLocal[reg] = val;

	Wire.beginTransmission(CS4272_ADDR);
	Wire.write(reg & 0xFF);
	Wire.write(val & 0xFF);
	Wire.endTransmission();
	return true;
}


// Initialize local registers to CS4272 reset status
void AudioControlCS4272_192k::initLocalRegs(void)
{
	regLocal[CS4272_MODE_CONTROL] = 0x00;
	regLocal[CS4272_DAC_CONTROL] = CS4272_DAC_CTRL_AUTO_MUTE;
	regLocal[CS4272_DAC_VOL] = CS4272_DAC_VOL_SOFT_RAMP(2) | CS4272_DAC_VOL_ATAPI(9);
	regLocal[CS4272_DAC_CHA_VOL] = 0x00;
	regLocal[CS4272_DAC_CHB_VOL] = 0x00;
	regLocal[CS4272_ADC_CTRL] = 0x00;
	regLocal[CS4272_MODE_CTRL2] = 0x00;
}



