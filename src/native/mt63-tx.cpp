
#include "mt63/mt63base.h"
#include <cstdint>
#include <vector>

#define WASM_EXPORT(name) __attribute__((export_name(#name)))

const double TX_LEVEL = -3.0;
const double SIG_LIMIT = 0.95;

const double TONE_LEVEL = -6.0;

const unsigned int SAMPLE_RATE = 8000;
const unsigned int BUFFER_SECONDS = 60;
const unsigned int BUFFER_SIZE = SAMPLE_RATE * BUFFER_SECONDS;

MT63tx Tx;

using AudioSampleType = float;
using DataType = uint8_t;

// Preallocate the output buffer with a specific size so that we can avoid
// costly reallocation of the backing storage as much as possible.
std::vector<AudioSampleType> outputBuffer(BUFFER_SIZE);
std::vector<DataType> inputBuffer;


int WASM_EXPORT(getSampleRate)getSampleRate()
{
	return SAMPLE_RATE;
}

void flushToBuffer(MT63tx *Tx)
{
	double mult = pow(10, TX_LEVEL / 20);

	for (auto i = 0; i < Tx->Comb.Output.Len; i++)
	{
		auto val = Tx->Comb.Output.Data[i] * mult;

		if (val > SIG_LIMIT) val = SIG_LIMIT;
		if (val < -SIG_LIMIT) val = -SIG_LIMIT;

		outputBuffer.push_back(val);
	}
}

void interleaveFlush(MT63tx *Tx, bool silent)
{
	for (auto i = 0; i < Tx->DataInterleave; ++i)
	{
		Tx->SendChar(0);
		if (!silent)
			flushToBuffer(Tx);
	}

	// We need at least two frames of silence to flush all the internal state
	// and clear the FFT window. We do three here just for good measure.
	// If we don't do this we'll end up with artifacts at the beginning and end
	// of transmissions.
	for (auto i = 0; i < 3; i++)
	{
		Tx->SendSilence();
		if (!silent)
			flushToBuffer(Tx);
	}
}

void sendTone(MT63tx *Tx, double seconds, int bandwidth, int center)
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

size_t WASM_EXPORT(transmit)transmit(int center, int bandwidth, int interleave)
{
	if (bandwidth != 500 && bandwidth != 1000 && bandwidth != 2000)
		return -1; // Invalid entry

	if (interleave < 0 || interleave > 1)
		return -1; // Invalid entry

	outputBuffer.clear();

	Tx.Preset(center, bandwidth, interleave);

	sendTone(&Tx, 1.5, bandwidth, center);

	interleaveFlush(&Tx, true);

	for (DataType c : inputBuffer)
	{
		if (c > 0x7F)
		{
			c &= 0x7F;
			Tx.SendChar(0x7F);
			flushToBuffer(&Tx);
		}

		Tx.SendChar(c);
		flushToBuffer(&Tx);
	}

	interleaveFlush(&Tx, false);

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
