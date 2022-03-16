
#include <cstdint>
#include <vector>

#include "mt63/mt63base.h"
#include "resampler.h"


#define WASM_EXPORT(name) __attribute__((export_name(#name)))

const double TX_LEVEL = -3.0;
const double SIG_LIMIT = 0.95;

const double TONE_LEVEL = -6.0;

const unsigned int SAMPLE_RATE = 8000;
const unsigned int BUFFER_SECONDS = 60;
const unsigned int BUFFER_SIZE = SAMPLE_RATE * BUFFER_SECONDS;

MT63tx tx;

unsigned int outputSampleRate = SAMPLE_RATE;
Resampler *resampler = new Resampler(SAMPLE_RATE, outputSampleRate, 64, 32);

using AudioSampleType = float;
using DataType = uint8_t;

// Preallocate the output buffer with a specific size so that we can avoid
// costly reallocation of the backing storage as much as possible.
std::vector<AudioSampleType> outputBuffer(BUFFER_SIZE);
std::vector<AudioSampleType> resampleBuffer(BUFFER_SIZE);
std::vector<DataType> inputBuffer;


void flushToBuffer(MT63tx *tx)
{
	double mult = pow(10, TX_LEVEL / 20);

	for (auto i = 0; i < tx->Comb.Output.Len; i++)
	{
		auto val = tx->Comb.Output.Data[i] * mult;

		if (val > SIG_LIMIT) val = SIG_LIMIT;
		if (val < -SIG_LIMIT) val = -SIG_LIMIT;

		outputBuffer.push_back(val);
	}
}

void interleaveFlush(MT63tx *tx, bool silent)
{
	for (auto i = 0; i < tx->DataInterleave; ++i)
	{
		tx->SendChar(0);
		if (!silent)
			flushToBuffer(tx);
	}

	// We need at least two frames of silence to flush all the internal state
	// and clear the FFT window. We do three here just for good measure.
	// If we don't do this we'll end up with artifacts at the beginning and end
	// of transmissions.
	for (auto i = 0; i < 3; i++)
	{
		tx->SendSilence();
		if (!silent)
			flushToBuffer(tx);
	}
}

void sendTone(MT63tx *tx, double seconds, int bandwidth, int center)
{
	const double frameSize = 400;
	int numFrames = SAMPLE_RATE * seconds / frameSize;

	double w1 = 2.0f * M_PI * (center - bandwidth / 2.0) / SAMPLE_RATE;
	double w2 = 2.0f * M_PI * (center + 31.0 * bandwidth / 64.0) / SAMPLE_RATE;
	double phi1 = 0.0;
	double phi2 = 0.0;

	double mult = pow(10, TONE_LEVEL / 20);

	for (int i = 0; i < numFrames; i++)
	{
		for (int j = 0; j < frameSize; j++)
		{
			double sample = (0.5 * cos(phi1) + 0.5 * cos(phi2)) * mult;

			if (i == 0)
				sample *= cos((1.0 - j / frameSize) * M_PI_2);

			if (i == numFrames - 1)
				sample *= cos((j / frameSize) * M_PI_2);

			outputBuffer.push_back(sample);

			phi1 += w1;
			if (phi1 > M_PI * 2.0) phi1 -= M_PI * 2.0;

			phi2 += w2;
			if (phi2 > M_PI * 2.0) phi2 -= M_PI * 2.0;
		}
	}
}

void WASM_EXPORT(setSampleRate)setSampleRate(unsigned int sampleRate)
{
	delete resampler;
	resampler = new Resampler(SAMPLE_RATE, sampleRate, 64, 32);
	outputSampleRate = sampleRate;
}

size_t WASM_EXPORT(transmit)transmit(int center, int bandwidth, int interleave)
{
	if (bandwidth != 500 && bandwidth != 1000 && bandwidth != 2000)
		return -1; // Invalid entry

	if (interleave < 0 || interleave > 1)
		return -1; // Invalid entry

	outputBuffer.clear();

	tx.Preset(center, bandwidth, interleave);

	sendTone(&tx, 1.5, bandwidth, center);

	interleaveFlush(&tx, true);

	for (DataType c : inputBuffer)
	{
		if (c > 0x7F)
		{
			c &= 0x7F;
			tx.SendChar(0x7F);
			flushToBuffer(&tx);
		}

		tx.SendChar(c);
		flushToBuffer(&tx);
	}

	interleaveFlush(&tx, false);

	if (outputSampleRate != SAMPLE_RATE)
	{
		resampleBuffer.assign(outputBuffer.begin(), outputBuffer.end());
		outputBuffer.clear();
		resampler->reset();
		resampler->process(&resampleBuffer, &outputBuffer);
		resampler->flush(&outputBuffer);
	}

	return outputBuffer.size();
}

AudioSampleType *WASM_EXPORT(getOutputBuffer)getOutputBuffer()
{
	return outputBuffer.data();
}

const unsigned char *WASM_EXPORT(getInputBuffer)getInputBuffer(size_t length)
{
	inputBuffer.resize(length);
	return inputBuffer.data();
}
