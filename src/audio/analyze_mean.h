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
	bool available(void) {
		return count > 0;
	}
	float read(void);
	virtual void update(void);
};

#endif

