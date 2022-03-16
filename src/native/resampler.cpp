#include <vector>
#include <cmath>

#include "resampler.h"

// Function to calculate a Lanczos kernel with parameter a at x
static inline float lanczos(double a, double x)
{
	if (x == 0)
		return 1.0f;

	return (a * sin(M_PI * x) * sin(M_PI * x / a)) / (M_PI * M_PI * x * x);
}

Resampler::Resampler(int from, int to, int quality, int memory)
{
	ratio = (double)to / (double)from;
	step = (double)from / (double)to;

	if (ratio >= 1)
		scale = 1.0;
	else
		scale = ratio;

	windowSize = quality / scale;

	kernelSize = memory * quality;
	kernelScale = kernelSize / windowSize;

	kernel.resize(kernelSize + 1);
	for (auto i = 0; i < kernelSize + 1; i++)
		kernel[i] = lanczos(quality, (double)i / (double)kernelSize * (double)quality);

	history.resize(ceil(windowSize) * 2 + ceil(step));

	reset();
}

void Resampler::reset(void)
{
	position = 0;
	history.assign(history.size(), 0);
}

void Resampler::flush(std::vector<SampleType> *out)
{
	std::vector<SampleType> zeros(windowSize + step, 0);
	process(&zeros, out);
}

void Resampler::process(std::vector<SampleType> *in, std::vector<SampleType> *out)
{
	while (position + step < in->size() - windowSize)
	{
		double outSample = 0.0;

		long int start = -windowSize + 1;
		long int end = windowSize;
		long int sampleIndex = position + start;

		double kernelIndex = (sampleIndex - position) * kernelScale;

		for (auto i = start; i < end; i++)
		{
			long int kernelIndexAbs = fabs(kernelIndex);
			double kernelFrac = fabs(kernelIndex) - kernelIndexAbs;

			double sample;
			if (sampleIndex >= 0)
				sample = (*in)[sampleIndex];
			else
				sample = history[sampleIndex + history.size()];

			outSample += sample * kernel[kernelIndexAbs] * (1.0 - kernelFrac);
			outSample += sample * kernel[kernelIndexAbs + 1] * kernelFrac;

			kernelIndex += kernelScale;
			sampleIndex++;
		}

		out->push_back(outSample * scale);
		position += step;
	}

	position -= in->size();

	auto histSize = std::min<int>(in->size(), history.size());

	for (auto i = 0; i < history.size() - histSize; i++)
		history[i] = history[history.size() - histSize + i];

	for (auto i = -histSize; i < 0; i++)
		history[history.size() + i] = (*in)[in->size() + i];
}
