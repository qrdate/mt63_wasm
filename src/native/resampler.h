#ifndef RESIMPLER_H
#define RESIMPLER_H

#include <vector>

class Resampler
{
public:
	using SampleType = float;

	Resampler(int from, int to, int quality = 128, int memory = 64);

	void reset(void);
	void process(std::vector<SampleType> *in, std::vector<SampleType> *out);
	void flush(std::vector<SampleType> *out);

private:
	std::vector<double> history;
	double position;

	std::vector<double> kernel;
	int kernelSize;
	double kernelScale;

	double ratio;
	double step;
	double windowSize;

	double scale;
};

#endif