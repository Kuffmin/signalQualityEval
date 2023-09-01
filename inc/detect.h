extern "C" __declspec(dllexport) int __stdcall signal_quality_eval(const char* modelName, const char* imgFilePath, const float conf_thresh);
extern "C" __declspec(dllexport) int __stdcall getVersion(char* version, int len);

#define DEMENSIONS 7
#define INPUT_WIDTH 640
#define INPUT_HEIGHT 640
#define CONFIDENCE_THRESHOLD 0.5
#define SCORE_THRESHOLD 0.2
#define NMS_THRESHOLD 0.5

struct Detection
{
	int class_id;
	float confidence;
	cv::Rect box;
};

/*

*Summary: ִ����������

*Parameters:

*     modelName:���ģ�͵�·��
*     imgFileName:���ͼ���·��
*     conf_thresh:���Ŷ���ֵ����Ϊ0.8
*Return : �������۷���0��100

*/
int signal_quality_eval(const char* modelName, const char* imgFileName, const float conf_thresh);

/*
	��ȡ�㷨�汾��
*/
int getVersion(char* version, int len);
