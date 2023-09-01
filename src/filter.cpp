#include "filter.h"

// �ο�matlab filter(b,a,x)�ĵ�: a(1)y(n)=b(1)x(n)-a(2)y(n-1)
void filter(std::vector<float>& x, float b1, float a1, float a2, std::vector<float>& y)
{
	float yTemp = 0.0f;

	// y(n-1)=0,y(1)��������
	yTemp= b1 * x[0] - a2 * 0;
	y.push_back(yTemp);

	// ����y(2)...y(n)
	for (int i = 1; i < x.size(); i++)
	{
		yTemp = b1 * x[i] - a2 * y[i - 1];
		y.push_back(yTemp);
	}
}