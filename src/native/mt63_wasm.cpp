
#include "mt63/mt63base.h"
#include <vector>
#include <string>

#define WASM_EXPORT(name) __attribute__((export_name(#name)))


const double TX_LEVEL = -3.0;
const double SIG_LIMIT = 0.95;

const double TONE_LEVEL = -6.0;

const unsigned int SAMPLE_RATE = 8000;
const unsigned int BUFFER_SECONDS = 60;
const unsigned int BUFFER_SIZE = SAMPLE_RATE * BUFFER_SECONDS;

using AudioSampleType = float;


MT63tx Tx;

std::vector<AudioSampleType> encodeAudio(BUFFER_SIZE);
unsigned int encodeAudioSize = 0;

std::string encodeText;


// MT63rx Rx;


int WASM_EXPORT(getSampleRate)getSampleRate()
{
	return SAMPLE_RATE;
}


void resetEncoderBuffer()
{
	encodeAudio[0] = 0;
	encodeAudioSize = 0;
}

void flushToBuffer(MT63tx *Tx)
{
	double mult = pow(10, TX_LEVEL / 20);

	while (encodeAudioSize + Tx->Comb.Output.Len > encodeAudio.size())
		encodeAudio.resize(encodeAudio.size() * 2);

	for (auto i = 0; i < Tx->Comb.Output.Len; ++i)
	{
		auto val = Tx->Comb.Output.Data[i] * mult;

		if (val > SIG_LIMIT) val = SIG_LIMIT;
		if (val < -SIG_LIMIT) val = -SIG_LIMIT;

		encodeAudio[encodeAudioSize++] = static_cast<AudioSampleType>(val);
	}
}

void interleaveFlush(MT63tx *Tx, bool output)
{
	for (auto i = 0; i < Tx->DataInterleave; ++i)
	{
		Tx->SendChar(0);
		if (output)
			flushToBuffer(Tx);
	}

	for (auto i = 0; i < 3; i++)
	{
		Tx->SendSilence();
		if (output) flushToBuffer(Tx);
	}
}

void sendTone(MT63tx *Tx, double seconds, int bandwidth, int center)
{
	const double block_size = 400;
	int num_blocks = SAMPLE_RATE * seconds / block_size;

	double w1 = 2.0f * M_PI * (center - bandwidth / 2.0) / SAMPLE_RATE;
	double w2 = 2.0f * M_PI * (center + 31.0 * bandwidth / 64.0) / SAMPLE_RATE;
	double phi1 = 0.0;
	double phi2 = 0.0;

	double mult = pow(10, TONE_LEVEL / 20);

	for (int i = 0; i < num_blocks; i++)
	{
		for (int j = 0; j < block_size; j++)
		{
			double sample = (0.5 * cos(phi1) + 0.5 * cos(phi2)) * mult;

			if (i == 0)
				sample *= cos((1.0 - j / block_size) * M_PI_2);

			if (i == num_blocks - 1)
				sample *= cos((j / block_size) * M_PI_2);

			encodeAudio[encodeAudioSize++] = sample;

			phi1 += w1;
			if(phi1 > M_PI_2) phi1 -= M_PI_2;

			phi2 += w2;
			if(phi2 > M_PI_2) phi2 -= M_PI_2;
		}
	}
}

size_t WASM_EXPORT(encode)encode(int center, int bandwidth, int interleave)
{
	if (bandwidth != 500 && bandwidth != 1000 && bandwidth != 2000)
	{
		return 0; // Invalid entry
	}
	if (interleave < 0 || interleave > 1)
	{
		return 0; // Invalid entry
	}

	resetEncoderBuffer();

	Tx.Preset(center, bandwidth, interleave);

	sendTone(&Tx, 1.5, bandwidth, center);

	interleaveFlush(&Tx, false);

	for (unsigned char c : encodeText)
	{
		if (c > 127)
		{
			c &= 127;
			Tx.SendChar(127);
			flushToBuffer(&Tx);
		}

		Tx.SendChar(c);
		flushToBuffer(&Tx);
	}

	interleaveFlush(&Tx, true);

	return encodeAudioSize;
}

AudioSampleType *WASM_EXPORT(getEncodeAudio)getEncodeAudio()
{
	return encodeAudio.data();
}

const char *WASM_EXPORT(getEncodeText)getEncodeText(size_t length)
{
	encodeText.resize(length);
	return encodeText.data();
}


/*std::string lastString;

int escape = 0;
double sqlVal = 8.0;

bool tailExists = false;
double lastOutput = 0.0;
double lastWeight = 0.0;

std::vector<float> resampleBuffer(10);

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
}*/