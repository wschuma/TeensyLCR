#ifndef analyze_mean_h_
#define analyze_mean_h_

#include "Arduino.h"
#include "AudioStream.h"

class AudioAnalyzeMean : public AudioStream
{
private:
	audio_block_t *inputQueueArray[1];
	int64_t accum;
	uint32_t count;

public:
	AudioAnalyzeMean(void) : AudioStream(1, inputQueueArray) {
		accum = 0;
		count = 0;
	}
	uint32_t available(void) {
		return count;
	}
	float read(void);
	virtual void update(void);
};

#endif

