extern "C" __declspec(dllexport) float __stdcall signal_quality_eval(const char* modelName, const char* imgFileName, const char* BFDfileName, const char* PKfileName, const float thresh);
extern "C" __declspec(dllexport) int __stdcall getVersion(char* version, int len);

#define DEMENSIONS 8
#define INPUT_WIDTH 640
#define INPUT_HEIGHT 640
#define CONFIDENCE_THRESHOLD 0.2
#define SCORE_THRESHOLD 0.2
#define NMS_THRESHOLD 0.5
#define OPENCV_DEBUG 1

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
*     BFDfileName:���BFD�ļ���·��
*     PKfileName:���PK�ļ���·��
*     conf_thresh:���Ŷ���ֵ����Ϊ0.75
*Return : �������۷���0��100

*/
float signal_quality_eval(const char* modelName, const char* imgFileName, const char* BFDfileName, const char* PKfileName, const float thresh);

/*
	��ȡ�㷨�汾��
*/
int getVersion(char* version, int len);
