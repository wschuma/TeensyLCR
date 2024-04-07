

#ifndef control_cs4272_192k_h_
#define control_cs4272_192k_h_

#include "AudioControl.h"

class AudioControlCS4272_192k : public AudioControl
{
public:
	bool enable(void);
	bool disable(void) { return false; }
	bool volume(float n) { return volumeInteger(n * 127 + 0.499f); }
	bool inputLevel(float n) { return false; }
	bool inputSelect(int n) { return false; }

	bool volume(float left, float right);
	bool dacVolume(float n) { return volumeInteger(n * 127 + 0.499f); }
	bool dacVolume(float left, float right);

	bool muteOutput(void);
	bool unmuteOutput(void);

	bool muteInput(void);
	bool unmuteInput(void);

	bool enableDither(void);
	bool disableDither(void);

protected:
	bool write(unsigned int reg, unsigned int val);
	bool volumeInteger(unsigned int n); // range: 0x0 to 0x7F
	
	uint8_t regLocal[8];

	void initLocalRegs(void);
};

// For sample rate ratio select (only single speed tested)
#define CS4272_RATIO_SINGLE 0
#define CS4272_RATIO_DOUBLE 2
#define CS4272_RATIO_QUAD 3

#endif
