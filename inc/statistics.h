#include <math.h>
#include <vector>
#include <numeric>

// ���
float sum(std::vector<float>& input);

// �����ֵ
float mean(std::vector<float>& input);

// ���㷽��
float var(std::vector<float>& input, int flag);

// ���������
float RMS(std::vector<float>& input);

// ������
void diff(std::vector<float>& input, std::vector<float>& diffValue);

// ����������
float sampEn2_m2(std::vector<float>& X, int N, float r);