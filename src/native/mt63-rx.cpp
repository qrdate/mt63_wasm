
#include "mt63/mt63base.h"
#include <cstdint>
#include <vector>
#include <string>

#define WASM_EXPORT(name) __attribute__((export_name(#name)))

// NOT DONE YET, SORRY!

/*
const unsigned int SAMPLE_RATE = 8000;

std::string lastString;

int escape = 0;
double sqlVal = 8.0;

bool tailExists = false;
double lastOutput = 0.0;
double lastWeight = 0.0;

using AudioSampleType = float;
using DataType = uint8_t;

std::vector<AudioSampleType> resampleBuffer(10);

MT63rx Rx;


int WASM_EXPORT(getSampleRate)getSampleRate()
{
	return SAMPLE_RATE;
}

size_t downSample(float *input, size_t bufferLength, double from, double to, float *output)
{
	const auto ratioWeight = from / to;
	size_t outputOffset = 0;

	if (bufferLength > 0)
	{
		auto buffer = input;
		double weight = 0;
		double output0 = 0.0;
		size_t actualPosition = 0;
		size_t amountToNext = 0;
		bool alreadyProcessedTail = !tailExists;
		tailExists = false;
		const auto outputBuffer = output;
		size_t currentPosition = 0;

		do
		{
			if (alreadyProcessedTail)
			{
				weight = ratioWeight;
				output0 = 0;
			}
			else
			{
				weight = lastWeight;
				output0 = lastOutput;
				alreadyProcessedTail = true;
			}

			while (weight > 0 && actualPosition < bufferLength)
			{
				amountToNext = 1 + actualPosition - currentPosition;
				if (weight >= amountToNext)
				{
					output0 += buffer[actualPosition++] * amountToNext;
					currentPosition = actualPosition;
					weight -= amountToNext;
				}
				else
				{
					output0 += buffer[actualPosition] * weight;
					currentPosition += weight;
					weight = 0;
					break;
				}
			}

			if (weight <= 0)
			{
				outputBuffer[outputOffset++] = output0 / ratioWeight;
			}
			else
			{
				lastWeight = weight;
				lastOutput = output0;
				tailExists = true;
				break;
			}
		}
		while (actualPosition < bufferLength);
	}

	return outputOffset;
}


void WASM_EXPORT(initRx)initRx(int center, int bandwidth, int interleave, int integration, double squelch)
{
	Rx.Preset(center, bandwidth, interleave, integration, nullptr);
	sqlVal = squelch;
}

const char *WASM_EXPORT(processAudio)processAudio(float *samples, int len)
{
	float_buff inBuff;
	inBuff.Data = samples;
	inBuff.Len = len;
	inBuff.Space = len;

	Rx.Process(&inBuff);
	if (Rx.FEC_SNR() < sqlVal)
	{
		return "";
	}

	lastString = std::string();
	for (auto i = 0; i < Rx.Output.Len; ++i)
	{
		auto c = Rx.Output.Data[i];
		if ((c < 8) && escape == 0)
		{
			continue;
		}
		if (c == 127)
		{
			escape = 1;
			continue;
		}
		if (escape)
		{
			c += 128;
			escape = 0;
		}
		lastString.push_back(c);
	}

	return lastString.c_str();
}

// Note: for reasons that I haven't been able to track down, len must be
// an exact multiple of (sampleRate / 8000) -- so if your sample rate is
// 48000 then it needs to be evenly divisible by 6. Otherwise you end up
// with output that isn't always the same length and realloc calls elsewhere
// in the code blow up and die.
const char *WASM_EXPORT(processAudioResample)processAudioResample(float *samples, size_t sampleRate, size_t len)
{
	auto ratioWeight = sampleRate / SAMPLE_RATE;

	if (ratioWeight == 1)
		return processAudio(samples, len);
	else if (ratioWeight < 1)
		return NULL;

	// We need to downsample
	size_t maxOutputSize = static_cast<int>(len / ratioWeight) + 10;
	if (resampleBuffer.size() < maxOutputSize)
		resampleBuffer.resize(maxOutputSize);

	const auto newLen = downSample(samples, len, sampleRate, SAMPLE_RATE, &resampleBuffer[0]);

	return processAudio(&resampleBuffer[0], newLen);
}
*/