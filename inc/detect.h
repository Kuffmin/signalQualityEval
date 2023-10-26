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

*Summary: 执行质量评价

*Parameters:

*     modelName:存放模型的路径
*     imgFileName:存放图像的路径
*     BFDfileName:存放BFD文件的路径
*     PKfileName:存放PK文件的路径
*     conf_thresh:置信度阈值，设为0.75
*Return : 质量评价分数0—100

*/
float signal_quality_eval(const char* modelName, const char* imgFileName, const char* BFDfileName, const char* PKfileName, const float thresh);

/*
	获取算法版本号
*/
int getVersion(char* version, int len);
