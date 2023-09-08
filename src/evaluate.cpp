#include <cmath>
#include <iostream>
#include <fstream>
#include <windows.h>
#include <vector>
#include "statistics.h"
#include "kurtosis.h"
#include "bandpower.h"
#include "filter.h"
#include "evaluate.h"
#include <opencv2/opencv.hpp>

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

float evaluate(std::string BFDname, std::string PKname)
{
	// 读取.BFD文件和.PK文件
	std::ifstream bfd;
	std::ifstream peak;
	bfd.open(BFDname, std::ios::binary);
	peak.open(PKname, std::ios::binary);

	std::vector<float> data;
	std::vector<int> time;

	if (!bfd)
	{
		std::cerr << "Failed to open BFD file." << std::endl;
		return -1;
	}

	if (!peak)
	{
		std::cerr << "Failed to open PEAK file." << std::endl;
		return -1;
	}

	float d = 0.0f;
	while (bfd.peek() != EOF) {
		bfd.read((char*)&d, sizeof(d));
		data.push_back(d);
	}
	bfd.close();

	int t = 0.0f;
	while (peak.peek() != EOF) {
		peak.read((char*)&t, sizeof(t));
		time.push_back(t);
	}
	peak.close();

	// 预处理数据
	int channelNum = 36;
	int sampleRate = 1000;
	std::vector<std::vector<float>> data1(36);
	preprocessingDataNoFilter(data, channelNum, sampleRate, data1);

	// 记录36个特征值
	std::vector<float> Fea;
	std::vector<float> temp;
	int i = 0, j = 0;

	// 特征1，信噪比再求mad平均绝对偏差
	float value = 0.0f;
	std::vector<float> variance;
	for (i = 0; i < channelNum; i++)
	{
		value = var(data1[i], 1);
		variance.push_back(value);
	}

	std::vector<std::vector<float>> dataAbs(36);
	for (i = 0; i < channelNum; i++)
	{
		for (j = 0; j < data1[i].size(); j++)
		{
			dataAbs[i].push_back(abs(data1[i][j]));
		}
	}
	std::vector<float> varianceAbs;
	for (i = 0; i < channelNum; i++)
	{
		value = var(dataAbs[i], 1);
		varianceAbs.push_back(value);
	}

	std::vector<float> datasnr;
	for (i = 0; i < channelNum; i++)
	{
		datasnr.push_back(variance[i] / varianceAbs[i]);
	}
	float datasnrMean = mean(datasnr);
	float diffSum = 0.0f;
	for (i = 0; i < channelNum; i++)
	{
		temp.push_back(abs(datasnr[i] - datasnrMean));
	}
	Fea.push_back(mean(temp));
	temp.clear();

	// 特征2，每个通道序列方差/各自绝对值的方差，得到36个数值，再求36个数值的均方根
	Fea.push_back(RMS(datasnr));
	dataAbs.clear();
	varianceAbs.clear();

	// 特征3，信噪比标准差
	Fea.push_back(sqrtf(var(datasnr, 1)));

	// 特征4，先求各通道的峰度，再求和
	std::vector<float> kur;
	for (i = 0; i < channelNum; i++)
	{
		float kurValue = 0.0f;
		kurtosis(data1[i], &kurValue);
		kur.push_back(kurValue);
	}
	float kurSum = sum(kur);
	Fea.push_back(kurSum);

	// 特征5，样本熵求和
	std::vector<float> datasampen;
	float sampenValue = 0.0f;
	float sampenMean = 0.0f;
	for (i = 0; i < channelNum; i++)
	{
		sampenValue = sampEn2_m2(data1[i], data1[i].size(), 0.15f);
		datasampen.push_back(sampenValue);
	}
	Fea.push_back(sum(datasampen));

	// 特征6，36通道msd的mad平均绝对偏差
	float msdSum = 0.0f;
	std::vector<float> msd;
	std::vector<std::vector<float>> dataDiff(36);
	for (i = 0; i < channelNum; i++)
	{
		diff(data1[i], dataDiff[i]);
	}
	for (i = 0; i < channelNum; i++)
	{
		msdSum = 0.0f;
		for (j = 0; j < dataDiff[i].size(); j++)
		{
			msdSum += dataDiff[i][j] * dataDiff[i][j];
		}
		msd.push_back(msdSum);
	}
	dataDiff.clear();

	float msdMean = mean(msd);
	for (i = 0; i < channelNum; i++)
	{
		temp.push_back(abs(msd[i] - msdMean));
	}
	Fea.push_back(mean(temp));
	temp.clear();

	// 特征7，36通道信号两个频带的功率比值的msd
	std::vector<float> psd1, psd2, rfb1;
	bandpower(data1, 1, 5, psd1);
	bandpower(data1, 0, 45, psd2);
	for (i = 0; i < channelNum; i++)
	{
		rfb1.push_back(psd1[i] / psd2[i]);
	}

	std::vector<float> rfb1Diff;
	diff(rfb1, rfb1Diff);
	for (i = 0; i < rfb1Diff.size(); i++)
	{
		temp.push_back(rfb1Diff[i] * rfb1Diff[i]);
	}
	Fea.push_back(sum(temp));
	temp.clear();

	// 特征8，36通道样本熵的均值
	Fea.push_back(mean(datasampen));

	// 特征9（新增），高阶统计均值
	std::vector<float> skew;
	for (i = 0; i < channelNum; i++)
	{
		float kurValue = 0.0f;
		skewness(data1[i], &kurValue);
		skew.push_back(kurValue);
	}
	std::vector<float> hosSQI;
	for (i = 0; i < channelNum; i++)
	{
		hosSQI.push_back(kur[i] * abs(skew[i]) / 5);
	}
	Fea.push_back(mean(hosSQI));

	// 特征10（新增），QRS波群相对能量coa
	std::vector<std::vector<float>> dataQRS(36);
	for (i = 0; i < channelNum; i++)
	{
		for (j = 0; j < data1[i].size(); j++)
		{
			if (j >= time[2] && j <= time[4])
			{
				dataQRS[i].push_back(data1[i][j]);
			}
		}
	}
	std::vector<float> dataQRSSum;
	std::vector<float> data1Sum;
	std::vector<float> eSQI;
	float oneChannelSum = 0.0f;
	for (i = 0; i < channelNum; i++)
	{
		oneChannelSum = 0.0f;
		for (j = 0; j < dataQRS[i].size(); j++)
		{
			oneChannelSum += dataQRS[i][j] * dataQRS[i][j];
		}
		dataQRSSum.push_back(oneChannelSum);
	}
	for (i = 0; i < channelNum; i++)
	{
		oneChannelSum = 0.0f;
		for (j = 0; j < data1[i].size(); j++)
		{
			oneChannelSum += data1[i][j] * data1[i][j];
		}
		data1Sum.push_back(oneChannelSum);
	}
	for (i = 0; i < channelNum; i++)
	{
		eSQI.push_back(dataQRSSum[i] / data1Sum[i]);
	}
	Fea.push_back(sqrtf(var(eSQI,1))/mean(eSQI));

	// 特征11，样本熵的均方根
	Fea.push_back(RMS(datasampen));

	// 特征12，36通道msd的标准差
	Fea.push_back(sqrtf(var(msd, 1)));

	// 特征13，高阶统计求和
	Fea.push_back(sum(hosSQI));

	// 特征14，36通道信噪比的标准差除以均值
	float datasnrStd = sqrtf(var(datasnr, 1));
	Fea.push_back(datasnrStd / datasnrMean);

	// 特征15，平坦度方差最大值
	std::vector<std::vector<float>> dataFlat(36);
	std::vector<float> dataFlatVar;
	for (i = 0; i < channelNum; i++)
	{
		for (j = 0; j < data1[i].size(); j++)
		{
			if (j <= time[0] || (j >= time[4] && j <= time[5]) || j >= time[7])
			{
				dataFlat[i].push_back(data1[i][j]);
			}
		}
		dataFlatVar.push_back(var(dataFlat[i], 1));
	}
	Fea.push_back(*max_element(dataFlatVar.begin(), dataFlatVar.end()));

	// 特征16，36通道信号两个频带的功率比值的mad平均绝对偏差
	float rfb1Mean = mean(rfb1);
	for (i = 0; i < channelNum; i++)
	{
		temp.push_back(abs(rfb1[i] - rfb1Mean));
	}
	Fea.push_back(mean(temp));
	temp.clear();

	// 特征17（新增），36通道峰度均值
	Fea.push_back(mean(kur));

	// 特征18，36通道样本熵mad平均绝对偏差
	for (i = 0; i < channelNum; i++)
	{
		temp.push_back(abs(datasampen[i] - sampenMean));
	}
	Fea.push_back(mean(temp));
	temp.clear();

	// 特征19，pcaSQI
	cv::Mat pts(0, data1[0].size(), cv::DataType<float>::type);
	for (i = 0; i < data1.size(); i++)
	{
		cv::Mat ptsTemp(1, data1[i].size(), cv::DataType<float>::type, data1[i].data());
		pts.push_back(ptsTemp);
	}
	cv::PCA pca_analysis(pts, cv::Mat(), cv::PCA::DATA_AS_ROW);
	std::vector <float> eigenVal;
	for (i = 0; i < channelNum; i++)
	{
		eigenVal.push_back(pca_analysis.eigenvalues.at<float>(i));
	}
	float totalEigenVal = sum(eigenVal);
	float explainedVal = 0.0f;
	for (i = 0; i < 3; i++)
	{
		explainedVal += eigenVal[i] / totalEigenVal * 100;
	}
	Fea.push_back(floor(explainedVal));

	// 特征20，36通道样本熵标准差
	Fea.push_back(sqrtf(var(datasampen, 1)));

	// 特征21（新增），36通道偏度均值
	Fea.push_back(mean(skew));

	// 特征22（新增），信噪比求和
	Fea.push_back(sum(datasnr));

	// 特征23，36通道msd的rms值
	Fea.push_back(RMS(msd));

	// 特征24（新增），36通道峰度coa
	Fea.push_back(sqrtf(var(kur, 1)) / mean(kur));

	// 特征25，信噪比均值
	Fea.push_back(mean(datasnr));

	// 特征26（新增），T/R幅值比
	std::vector<float> Tpeak;
	std::vector<float> Rpeak;
	int timeT = time[6];
	int timeP = time[3];
	float TpeakMax = 0.0f;
	float RpeakMax = 0.0f;
	for (i = 0; i < 36; i++)
	{
		Tpeak.push_back(abs(data1[i][timeT]));
		Rpeak.push_back(abs(data1[i][timeP]));
	}
	TpeakMax = *max_element(Tpeak.begin(), Tpeak.end());
	RpeakMax = *max_element(Rpeak.begin(), Rpeak.end());
	Fea.push_back(TpeakMax / RpeakMax);

	// 特征27，36通道偏度求和
	Fea.push_back(sum(skew));

	// 特征31，36通道信噪比峰峰值
	float snrMax = *max_element(datasnr.begin(), datasnr.end());
	float snrMin = *min_element(datasnr.begin(), datasnr.end());
	Fea.push_back(snrMax - snrMin);
	
	float OriScore = 0.0f;
	if (Fea.size() != beta.size())
	{
		return -1;
	}
	for (i = 0; i < Fea.size(); i++)
	{
		OriScore += Fea[i] * beta[i];
	}
	OriScore = OriScore + bias;
	float badScore = 100.0f / (1 + exp(-2 * OriScore));
	float goodScore = 100.0f - badScore;
	
	return goodScore;
}