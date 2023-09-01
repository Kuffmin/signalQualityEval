#include <math.h>
#include <vector>
#include <numeric>

// 求和
float sum(std::vector<float>& input);

// 计算均值
float mean(std::vector<float>& input);

// 计算方差
float var(std::vector<float>& input, int flag);

// 计算均方根
float RMS(std::vector<float>& input);

// 计算差分
void diff(std::vector<float>& input, std::vector<float>& diffValue);

// 计算样本熵
float sampEn2_m2(std::vector<float>& X, int N, float r);