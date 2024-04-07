#include <Arduino.h>
#include "analyze_mean.h"

void AudioAnalyzeMean::update(void)
{
	audio_block_t *block = receiveReadOnly();
	if (!block) {
		count++;
		return;
	}
	int16_t *p = block->data;
	int16_t *end = p + AUDIO_BLOCK_SAMPLES;
	int64_t sum = accum;
	do {
		int32_t n = *p++;
		sum += n;
	} while (p < end);
	accum = sum;
	count++;

	release(block);
}

float AudioAnalyzeMean::read(void)
{
	__disable_irq();
	int64_t sum = accum;
	accum = 0;
	uint32_t num = count;
	count = 0;
	__enable_irq();
	float mean = sum / (num * AUDIO_BLOCK_SAMPLES);
	return mean / 32767.0;
}
