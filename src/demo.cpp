#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <opencv2/opencv.hpp>
#include "detect.h"
#include <vector>

int main(int argc, char** argv)
{
	// ��ȡ�汾��
	char version[] = "";
	getVersion(version, 8);

	// ͳ�ƺ�ʱ
	LARGE_INTEGER freq, begin, end;

	// ����ģ��
	char* modelName = "./model/best.onnx";

	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&begin);

	// ��������
	char* imgFileName = "D:/1/*.png";
	const float conf_thresh = 0.8f;
	int result = signal_quality_eval(modelName, imgFileName, conf_thresh);

	QueryPerformanceCounter(&end);
	double costTime = (end.QuadPart - begin.QuadPart) * 1000.0f / freq.QuadPart;
	std::cout << "Detect process cost time:" << costTime << std::endl;
	std::cout << "Signal quality eval result is:" << result << std::endl;
	return 0;
}

/*
#include "hammingWindow.h"
#include "sigpack.h"
using namespace arma;
using namespace sp;

void preprocessingDataNoFilter(std::vector<float>& src, int channelNum, int sampleRate, std::vector<std::vector<float>>& dst)
{
    int time = floor(src.size() / (channelNum * sampleRate));

    for (int i = 0; i < time; i++)
    {
        for (int j = 0; j < channelNum; j++)
        {
            for (int k = 0; k < sampleRate; k++)
            {
                dst[j].push_back(src[i * 36 * 1000 + j * 1000 + k]);
            }
        }
    }
}

int main()
{
    // ��ȡ.BFD�ļ���.PK�ļ�
    std::ifstream bfd;
    std::ifstream peak;
    bfd.open("D:\\��³�������߷������\\H0004-��ɺɺ\\20220530 091407\\20220530 091407.BFD", std::ios::binary);
    peak.open("D:\\��³�������߷������\\H0004-��ɺɺ\\20220530 091407\\20220530 091407.PK", std::ios::binary);

    std::vector<float> data;
    std::vector<int> time;

    if (!bfd)
    {
        std::cerr << "Failed to open BFD file." << std::endl;
        return 1;
    }

    if (!peak)
    {
        std::cerr << "Failed to open PEAK file." << std::endl;
        return 1;
    }

    float d = 0.0f;
    while (bfd.peek() != EOF) {
        bfd.read((char*)&d, sizeof(d));
        data.push_back(d);
    }
    bfd.close();

    // Ԥ��������
    int channelNum = 36;
    int sampleRate = 1000;
    std::vector<std::vector<float>> data1(36);
    preprocessingDataNoFilter(data, channelNum, sampleRate, data1);

    int i = 0;
    int N = 1000, NFFT = 1000, Noverl = 500, fs = 1000;
    vec x(N), S(N), power(N / 2), psd(N / 2);

    // ��ֵ
    for (i = 0; i < N; i++)
    {
        x[i] = data1[0][i];
    }

    S = pwelch(x, NFFT, Noverl);
    
    power[0] = S[0];
    for (i = 1; i < N/2; i++)
    {
        power[i] = S[i]*2;
    }
    // powerΪ�����ף����ݹ����׻��㹦�����ܶ�
    // �����׺͹������ܶȵĻ����ϵ�ο���https://blog.csdn.net/frostime/article/details/120870904
    // ���ȶ�Hamming�����
    vec temp(NFFT), ham2(1000);
    float sum = 0.0f, sum2 = 0.0f;
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

    float sum45 = 0.0f;
    for (int i = 1; i < 6; i++)
    {
        sum45 += psd[i];
    }

    std::cout << psd << std::endl;

    return 0;
}


*/