#include "statistics.h"

// 求和
float sum(std::vector<float>& input)
{
	float s = 0.0f;
	for (int i = 0; i < input.size(); ++i)
	{
		s += input[i];
	}
	return s;
}

// 计算均值
float mean(std::vector<float>& input)
{
	return sum(input) / input.size();
}

// 计算方差
float var(std::vector<float>& input, int flag)
{
	float mean = sum(input) / input.size();

	float accum = 0.0f;
	for (int i = 0; i < input.size(); i++)
	{
		accum += (input[i] - mean) * (input[i] - mean);
	}

	return accum / (input.size() - flag);
}

// 计算信号的有效值，也就是均方根
float RMS(std::vector<float> &input)
{
	int i = 0;
	float sum = 0.0f;
	float RMS = 0.0f;
	int length = input.size();
	for (i = 0; i < length; ++i)
	{
		sum += input[i] * input[i];
	}
	
	RMS = sqrtf(sum / length);

	return RMS;
}

// 计算差分
void diff(std::vector<float> &input, std::vector<float> &diffValue)
{
	int i = 0;
	int length = input.size();
	for (i = 0; i < length - 1; ++i)
	{
		diffValue.push_back(input[i + 1] - input[i]);
	}
}

// 计算样本熵，m是窗口长度
double step(double* X, int N, int m, double r)
{
	double sum = 0.0f;
	int i = 0, j = 0, k = 0;
	int Bi = 0;
	for (i = 0; i <= N - m; i++)
	{
		Bi = 0;
		for (j = 0; j <= N - m; j++)
		{
			if (i != j)
			{
				// 找出最大差值
				double D = fabs(X[i] - X[j]);
				for (k = 1; k < m; k++)
				{
					double t = fabs(X[i + k] - X[j + k]);
					if (D < t)
					{
						D = t;
					}
				}

				if (D <= r)
				{
					Bi++;
				}
			}
		}
		sum += 1.0f * Bi / (N - m);
	}

	return sum / (N - m + 1);
}


double sampEn(double* X, int N, int m, double r)
{
	double B = step(X, N, m, r);
	if (B == 0)
	{
		return 0;
	}
	double A = step(X, N, m + 1, r);
	if (A == 0)
	{
		return 0;
	}
	return -log(A / B);
}

double sampEnFast(double* X, int N, int m, double r)
{
	int Ai = 0, Bi = 0;
	int i = 0, j = 0, k = 0;
	int loopSub1 = N - m;
	for (i = 0; i <= loopSub1; i++)
	{
		for (j = 0; j <= loopSub1; j++)
		{
			if (i != j)
			{
				double D = fabs(X[i] - X[j]);
				for (k = 0; k < m; k++)
				{
					double t = fabs(X[i + k] - X[j + k]);
					if (D < t)
					{
						D = t;
					}
				}
				if (D <= r)
					Bi++;
				
				if (i != loopSub1 && j != loopSub1)
				{
					double t = fabs(X[i + m] - X[j + m]);
					if (D < t)
						D = t;
					if (D <= r)
						Ai++;
				}
			}
		}
	}
	double B = 1.0f * Bi / (N - m) / (N - m + 1);
	double A = 1.0f * Ai / (N - m - 1) / (N - m);
	if (B == 0 || A == 0)
		return 0;
	return -log(A / B);

}

float sampEn2_m2(std::vector<float> &X, int N, float r)
{
	float std = sqrtf(var(X, 1));       // 计算标准差
	float thresh = std * r;
	int Ai = 0, Bi = 0;
	int LoopsSub1 = N - 2;
	for (int i = 0; i < LoopsSub1; i++)
	{
		for (int j = 0; j <= LoopsSub1; j++)
		{
			if (i != j)
			{
				float D = fabs(X[i] - X[j]);
				float t = fabs(X[i + 1] - X[j + 1]);
				if (D < t)
					D = t;
				if (D <= thresh)
					Bi++;
				if (i != LoopsSub1 && j != LoopsSub1)
				{
					float t = fabs(X[i + 2] - X[j + 2]);
					if (D < t)
						D = t;
					if (D <= thresh)
						Ai++;
				}
			}
		}
	}
	float B = 1.0f * Bi / (N - 2) / (N - 1);
	float A = 1.0f * Ai / (N - 3) / (N - 2);
	if (B == 0 || A == 0)
		return 0;
	return -log(A / B);
}