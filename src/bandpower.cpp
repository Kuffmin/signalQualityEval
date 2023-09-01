#include "bandpower.h"

void bandpower(std::vector<std::vector<float>>& input, int rangeLow, int rangeHigh, std::vector<float>& output)
{
    int i = 0;
    int N = 1000, NFFT = 1000, Noverl = 500, fs = 1000;
    vec x(N), S(N), power(N / 2), psd(N / 2);

    for (int num = 0; num < 36; num++)
    {
        // 赋值
        for (i = 0; i < N; i++)
        {
            x[i] = input[num][i];
        }

        // welch计算功率谱
        S = pwelch(x, NFFT, Noverl);

        power[0] = S[0];
        for (i = 1; i < N / 2; i++)
        {
            power[i] = S[i] * 2;
        }
        // power为功率谱，根据功率谱换算功率谱密度
        // 功率谱和功率谱密度的换算关系参考：https://blog.csdn.net/frostime/article/details/120870904
        // 首先对Hamming窗求和
        vec temp(NFFT), ham2(1000);
        float sum = 0.0f, sum2 = 0.0f, sumRange = 0.0f;
        for (i = 0; i < N; i++)
        {
            sum += hammingWindow[i];
        }
        sum2 = sum * sum;
        for (i = 0; i < power.size(); i++)
        {
            temp[i] = power[i] * sum2;
        }
        for (i = 0; i < N; i++)
        {
            ham2[i] = hammingWindow[i] * hammingWindow[i];
        }
        sum = 0.0f;
        for (i = 0; i < N; i++)
        {
            sum += ham2[i];
        }
        for (i = 0; i < N / 2; i++)
        {
            psd[i] = temp[i] / sum / fs;
        }

        for (int i = rangeLow; i <= rangeHigh; i++)
        {
            sumRange += psd[i];
        }
        output.push_back(sumRange);
    }
    
}