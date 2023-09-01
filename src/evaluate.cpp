#include <cmath>
#include <iostream>
#include <fstream>
#include <windows.h>
#include <vector>
#include "statistics.h"
#include "kurtosis.h"
#include "bandpower.h"
#include "filter.h"
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

int main(int argc, char** argv)
{
	// 读取.BFD文件和.PK文件
	std::ifstream bfd;
	std::ifstream peak;
	bfd.open("D:\\齐鲁数据离线分析结果\\H0004-刘珊珊\\20220530 091407\\20220530 091407.BFD", std::ios::binary);
	peak.open("D:\\齐鲁数据离线分析结果\\H0004-刘珊珊\\20220530 091407\\20220530 091407.PK", std::ios::binary);

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

	// 特征1，每个通道序列方差/各自绝对值的方差，得到36个数值，再求36个数值的均方根
	int i = 0, j = 0;
	float value = 0.0f;
	std::vector<float> variance;
	for (i = 0; i < channelNum; i++)
	{
		value = var(data1[i],1);
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
		value = var(dataAbs[i],1);
		varianceAbs.push_back(value);
	}

	std::vector<float> datasnr;
	for (i = 0; i < channelNum; i++)
	{
		datasnr.push_back(variance[i] / varianceAbs[i]);
	}
	Fea.push_back(RMS(datasnr));
	dataAbs.clear();
	varianceAbs.clear();

	// 特征2，先求各通道的峰度，再求和
	std::vector<float> kur;
	for (i = 0; i < channelNum; i++)
	{
		float kurValue = 0.0f;
		kurtosis(data1[i], &kurValue);
		kur.push_back(kurValue);
	}
	float kurSum = sum(kur);
	Fea.push_back(kurSum);


	// 特征3，信噪比再求mad平均绝对偏差
	float datasnrMean = mean(datasnr);
	float diffSum = 0.0f;
	for (i = 0; i < channelNum; i++)
	{
		temp.push_back(abs(datasnr[i] - datasnrMean));
	}
	Fea.push_back(mean(temp));
	temp.clear();

	// 特征4，36通道样本熵的均值
	std::vector<float> datasampen;
	float sampenValue = 0.0f;
	float sampenMean = 0.0f;
	for (i = 0; i < channelNum; i++)
	{
		sampenValue = sampEn2_m2(data1[i], data1[i].size(), 0.15);
		datasampen.push_back(sampenValue);
	}
	sampenMean = mean(datasampen);
	Fea.push_back(sampenMean);

	// 特征5，msd求和
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
	Fea.push_back(sum(msd));
	dataDiff.clear();

	// 特征6，每个通道序列方差/各自绝对值的方差，得到36个数值，再求36个数值的标准差
	float datasnrStd = sqrtf(var(datasnr, 1));
	Fea.push_back(datasnrStd);

	// 特征7，eSQI求和
	std::vector<std::vector<float>> dataQRS(36);
	for (i = 0; i < channelNum; i++)
	{
		for (j = 0; j <data1[i].size(); j++)
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
	Fea.push_back(sum(eSQI));

	// 特征8，平坦度方差最大值
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
		dataFlatVar.push_back(var(dataFlat[i],1));
	}
	Fea.push_back(*max_element(dataFlatVar.begin(), dataFlatVar.end()));

	// 特征9，样本熵的均方根
	Fea.push_back(RMS(datasampen));

	// 特征10，高阶统计
	std::vector<float> skew;
	for (i = 0; i < channelNum; i++)
	{
		float kurValue = 0.0f;
		skewness(data1[i], &kurValue);
		skew.push_back(kurValue);
	}
	float hosSQI = 0.0f;
	for (i = 0; i < channelNum; i++)
	{
		hosSQI += kur[i] * abs(skew[i]) / 5;
	}
	Fea.push_back(hosSQI);

	// 特征11，信噪比均值
	Fea.push_back(mean(datasnr));

	// 特征12，36通道msd的mad平均绝对偏差
	float msdMean = mean(msd);
	for (i = 0; i < channelNum; i++)
	{
		temp.push_back(abs(msd[i] - msdMean));
	}
	Fea.push_back(mean(temp));
	temp.clear();

	// 特征13，36通道样本熵mad平均绝对偏差
	for (i = 0; i < channelNum; i++)
	{
		temp.push_back(abs(datasampen[i] - sampenMean));
	}
	Fea.push_back(mean(temp));
	temp.clear();

	// 特征14，36通道msd的标准差
	Fea.push_back(sqrtf(var(msd,1)));

	// 特征15，pcaSQI
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
	Fea.push_back(explainedVal);

	// 特征16，36通道样本熵标准差
	Fea.push_back(sqrtf(var(datasampen, 1)));

	// 特征17，36通道的前150ms标准差的标准差
	std::vector<std::vector<float>> data150(36);
	std::vector<float> std150;
	for (i = 0; i < channelNum; i++)
	{
		for (j = 0; j < 150; j++)
		{
			data150[i].push_back(data1[i][j]);
		}
	}
	for (i = 0; i < channelNum; i++)
	{
		std150.push_back(sqrtf(var(data150[i], 1)));
	}
	Fea.push_back(sqrtf(var(std150, 1)));

	// 特征18，36通道msd的峰峰值
	float msdMax = *max_element(msd.begin(), msd.end());
	float msdMin = *min_element(msd.begin(), msd.end());
	Fea.push_back(msdMax - msdMin);

	// 特征19，36通道信噪比的标准差除以均值
	Fea.push_back(datasnrStd / datasnrMean);

	// 特征20，平坦度方差的mad平均绝对偏差
	float dataFlatVarMean = mean(dataFlatVar);
	for (i = 0; i < channelNum; i++)
	{
		temp.push_back(abs(dataFlatVar[i] - dataFlatVarMean));
	}
	Fea.push_back(mean(temp));
	temp.clear();

	// 特征21，36通道信号两个频带的功率比值的mad平均绝对偏差
	std::vector<float> psd1, psd2, rfb1;
	bandpower(data1, 1, 5, psd1);
	bandpower(data1, 0, 45, psd2);
	for (i = 0; i < channelNum; i++)
	{
		rfb1.push_back(psd1[i] / psd2[i]);
	}
	float rfb1Mean = mean(rfb1);
	for (i = 0; i < channelNum; i++)
	{
		temp.push_back(abs(rfb1[i] - rfb1Mean));
	}
	Fea.push_back(mean(temp));
	temp.clear();

	// 特征22，36通道msd的裕度因子
	for (i = 0; i < channelNum; i++)
	{
		temp.push_back(sqrtf(abs(msd[i])));
	}
	float msdTemp = pow(mean(temp), 2);
	temp.clear();
	for (i = 0; i < channelNum; i++)
	{
		temp.push_back(abs(msd[i]) / msdTemp);
	}
	Fea.push_back(*max_element(temp.begin(), temp.end()));
	temp.clear();

	// 特征23，样本熵求和
	Fea.push_back(sum(datasampen));

	// 特征24，ST段
	Fea.push_back(time[5] - time[4]);

	// 特征25，TT间期
	Fea.push_back(time[7] - time[5]);
	
	// 特征26，36通道偏度求和
	Fea.push_back(sum(skew));

	// 特征27，36通道msd的rms值
	Fea.push_back(RMS(msd));

	// 特征28，36通道的前150ms标准差的均方根
	Fea.push_back(RMS(std150));

	// 特征29，36通道信号两个频带的功率比值的msd
	std::vector<float> rfb1Diff;
	diff(rfb1, rfb1Diff);
	for (i = 0; i < rfb1Diff.size(); i++)
	{
		temp.push_back(rfb1Diff[i] * rfb1Diff[i]);
	}
	Fea.push_back(sum(temp));
	temp.clear();

	// 特征30，低通滤波，baSQI求和
	std::vector<std::vector<float>> dataBase(36);
	std::vector<float> RangeBase;
	std::vector<float> RangeQRS;
	for (i = 0; i < channelNum; i++)
	{
		filter(data1[i], 0.0503, 1, -0.9497, dataBase[i]);
	}
	for (i = 0; i < channelNum; i++)
	{
		RangeQRS.push_back(*max_element(dataQRS[i].begin(), dataQRS[i].end()) - *min_element(dataQRS[i].begin(), dataQRS[i].end()));
	}
	for (i = 0; i < channelNum; i++)
	{
		RangeBase.push_back(*max_element(dataBase[i].begin(), dataBase[i].end()) - *min_element(dataBase[i].begin(), dataBase[i].end()));
	}
	for (i = 0; i < channelNum; i++)
	{
		temp.push_back(RangeQRS[i] / RangeBase[i]);
	}
	Fea.push_back(sum(temp));
	temp.clear();

	// 特征31，36通道信噪比峰峰值
	float snrMax = *max_element(datasnr.begin(), datasnr.end());
	float snrMin = *min_element(datasnr.begin(), datasnr.end());
	Fea.push_back(snrMax - snrMin);

	// 特征32，36通道的前150ms标准差的均值
	Fea.push_back(mean(std150));

    // 特征33，36通道msd的均值
	Fea.push_back(mean(msd));

	// 特征34，36通道样本熵的裕度因子
	float value1 = 0.0f, value2 = 0.0f;
	for (i = 0; i < channelNum; i++)
	{
		temp.push_back(sqrtf(abs(datasampen[i])));
	}
	value2 = pow(mean(temp), 2);
	temp.clear();
	for (i = 0; i < channelNum; i++)
	{
		temp.push_back(abs(datasampen[i]));
	}
	value1 = *max_element(temp.begin(), temp.end());
	temp.clear();
	Fea.push_back(value1 / value2);

	std::vector<float> beta = { 0.151196285718451
,    -0.00196137417725240
,    0.0813611151968867
,    0.0127393713694126
,    -0.0405442406832322
,    0.0710209518624612
,    0.0442065767875395
,    0.117861737169923
,    0.0101499877734858
,    0.000551948421296800
,    0.136254267093236
,    -0.121915497538181
,    0.00306935118479216
,    -0.200791090808309
,    -0.408971085236686
,    -0.000148600385325702
,    0.0608161025604451
,    -0.181897376644663
,    0.0237800262424230
,    -0.0222960318187585
,    0.0375529954318381
,    0.170830616446587
,    0.331310115143318
,    0.00209878739035219
,    -0.00461713085035775
,    0.00310618811062469
,    0.383556185213415
,    0.108305795522957
,    0.0856110764403233
,    0.00163742682872403
,    -0.0446809323564580
,    0.0892710146631016
,    1.25838768546619
,    -0.00401530073249559 };
	float bias = 38.548309704351354;
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
	return 0;
}