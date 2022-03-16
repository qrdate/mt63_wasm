#include <vector>
#include <string>

#include "mt63/mt63base.h"
#include "resampler.h"


#define WASM_EXPORT(name) __attribute__((export_name(#name)))

const unsigned int SAMPLE_RATE = 8000;
const unsigned int BUFFER_SECONDS = 1;
const unsigned int BUFFER_SIZE = SAMPLE_RATE * BUFFER_SECONDS;

using AudioSampleType = float;
using DataType = uint8_t;

// Preallocate the output buffer with a specific size so that we can avoid
// costly reallocation of the backing storage as much as possible.
std::vector<AudioSampleType> inputBuffer(BUFFER_SIZE);
std::vector<AudioSampleType> resampleBuffer(BUFFER_SIZE);
std::vector<DataType> outputBuffer;
float_buff dspBuffer;

unsigned int inputSampleRate = SAMPLE_RATE;
Resampler *resampler = new Resampler(inputSampleRate, SAMPLE_RATE, 64, 32);


double sqlVal = 8.0;
MT63rx rx;


static void flushToBuffer(MT63rx *rx)
{
	static bool escape = false;

	for (auto i = 0; i < rx->Output.Len; ++i)
	{
		auto c = rx->Output.Data[i];

		if (c == 0)
			continue;

		if (c == 0x7F)
		{
			escape = true;
			continue;
		}

		if (escape)
		{
			c |= 0x80;
			escape = false;
		}

		outputBuffer.push_back(c);
	}
}


void WASM_EXPORT(setSampleRate)setSampleRate(unsigned int sampleRate)
{
	delete resampler;
	resampler = new Resampler(sampleRate, SAMPLE_RATE, 64, 32);
	inputSampleRate = sampleRate;
}

void WASM_EXPORT(initRx)initRx(int center, int bandwidth, int interleave, double squelch)
{
	rx.Preset(center, bandwidth, interleave);
	sqlVal = squelch;
}

size_t WASM_EXPORT(receive)receive()
{
	if (inputSampleRate != SAMPLE_RATE)
	{
		resampleBuffer.assign(inputBuffer.begin(), inputBuffer.end());
		inputBuffer.clear();
		resampler->reset();
		resampler->process(&resampleBuffer, &inputBuffer);
		resampler->flush(&inputBuffer);
	}

	dspBuffer.EnsureSpace(inputBuffer.size());
	dspBuffer.Len = inputBuffer.size();
	memcpy(dspBuffer.Data, inputBuffer.data(), inputBuffer.size() * sizeof(AudioSampleType));

	outputBuffer.clear();

	rx.Process(&dspBuffer);

	if ((rx.FEC_SNR() < sqlVal) || (!rx.SYNC_LockStatus()))
		return 0;

	flushToBuffer(&rx);

	return outputBuffer.size();
}

size_t WASM_EXPORT(flush)flush()
{
	dspBuffer.EnsureSpace(400);
	dspBuffer.Len = 400;
	memset(dspBuffer.Data, 0, 400 * sizeof(AudioSampleType));

	outputBuffer.clear();

	while (rx.SYNC_LockStatus())
	{
		rx.Process(&dspBuffer);
		flushToBuffer(&rx);
	}

	return outputBuffer.size();
}

float WASM_EXPORT(getSNR)getSNR()
{
	return rx.FEC_SNR();
}

bool WASM_EXPORT(getLock)getLock()
{
	return rx.SYNC_LockStatus();
}

DataType *WASM_EXPORT(getOutputBuffer)getOutputBuffer()
{
	return outputBuffer.data();
}

const AudioSampleType *WASM_EXPORT(getInputBuffer)getInputBuffer(size_t length)
{
	inputBuffer.resize(length);
	return inputBuffer.data();
}
