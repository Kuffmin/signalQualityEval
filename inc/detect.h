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

*Summary: 执行质量评价

*Parameters:

*     modelName:存放模型的路径
*     imgFileName:存放图像的路径
*     conf_thresh:置信度阈值，设为0.8
*Return : 质量评价分数0—100

*/
int signal_quality_eval(const char* modelName, const char* imgFileName, const float conf_thresh);

/*
	获取算法版本号
*/
int getVersion(char* version, int len);
