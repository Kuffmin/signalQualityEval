#include "kurtosis.h"

//int kurtosis(std::vector<float> &input, float* kurValue, float* skewness)
//{
//	int i = 0, j = 0, count = 0;
//	int length = input.size();
//	int* b = new int[length];
//	float exp = 0.0f;
//	float sum = 0.0f;
//	float y = 0.0f;
//
//	for (i = 0; i < length; ++i)
//	{
//		b[i] = -1;
//	}
//
//	for (i = 0; i < length; ++i)
//	{
//		count = 1;
//		for (j = i + 1; j < length; ++j)
//		{
//			if (abs(input.at(i) - input.at(j)) < EPSILON)
//			{
//				b[j] = 0;
//				count++;
//			}
//		}
//
//		if (b[i] != 0)
//		{
//			b[i] = count;
//		}
//	}
//
//	// 输出每个数出现的次数
//	for (i = 0; i < length; ++i)
//	{
//		if (b[i] != 0)
//		{
//			// 计算期望
//			exp += input.at(i) * (b[i] / float(length));
//		}
//	}
//
//	// 计算方差
//	for (i = 0; i < length; ++i)
//	{
//		sum += (input.at(i) * input.at(i)) * (b[i] / float(length));
//	}
//	float var = sum - (exp * exp);
//
//	// 倾斜度，求中心距y
//	for (i = 0; i < length; ++i)
//	{
//		y += ((input.at(i) - exp) * (input.at(i) - exp) * (input.at(i) - exp)) * (b[i] / float(length));
//	}
//	float skew = y / (sqrt(var) * var);
//
//	// 峰度
//	for (i = 0; i < length; ++i)
//	{
//		y += ((input.at(i) - exp) * (input.at(i) - exp) * (input.at(i) - exp) * (input.at(i) - exp)) * (b[i] / float(length));
//	}
//	float kurt = (y / (var * var)) - 3;
//
//	delete[] b;
//	
//	*kurValue = kurt;
//	*skewness = skew;
//	return 0;
//}

void kurtosis(std::vector<float> &input, float* kurValue)
{
	float variance = var(input, 0);
	float sigma = sqrtf(variance);
	float u = mean(input);
	float sum = 0.0f;

	for (int i = 0; i < input.size(); i++)
	{
		float value = (input[i] - u) / sigma;
		sum += value * value * value * value;
	}

	*kurValue= sum / input.size();
}

void skewness(std::vector<float>& input, float* skewValue)
{
	float variance = var(input, 0);
	float sigma = sqrtf(variance);
	float u = mean(input);
	float sum = 0.0f;

	for (int i = 0; i < input.size(); i++)
	{
		float value = (input[i] - u) / sigma;
		sum += value * value * value;
	}

	*skewValue = sum / input.size();
}

// 计算信号的功率(bandpower)
float bandpower(float RMS)
{
	return RMS * RMS;
}