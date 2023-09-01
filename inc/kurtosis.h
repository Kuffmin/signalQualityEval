#define EPSILON 0.000001

#include <vector>
#include "statistics.h"

// 计算峰度
void kurtosis(std::vector<float>& input, float* kurValue);

// 计算偏度
void skewness(std::vector<float>& input, float* skewValue);