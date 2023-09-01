#include <fstream>
#include <sstream>
#include <windows.h>
#include <opencv2/opencv.hpp>
#include "detect.h"

const std::vector<cv::Scalar> colors = { cv::Scalar(0,0,255),cv::Scalar(255,0,0),cv::Scalar(0,255,255) };

std::vector<std::string> load_class_list(std::string className)
{
	std::vector<std::string> class_list;
	std::ifstream file(className);
	std::string line;
	while (getline(file, line))
	{
		class_list.push_back(line);
	}

	return class_list;
}

void load_net(std::string modelName, bool is_cuda, cv::dnn::Net& net)
{
	auto result = cv::dnn::readNet(modelName);
	if (is_cuda)
	{
		std::cout << "use CUDA\n";
		result.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
		result.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA_FP16);
	}
	else
	{
		std::cout << "Runing on CPU\n";
		result.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
		result.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
	}
	net = result;
}

cv::Mat format_yolov5(const cv::Mat& source)
{
	int col = source.cols;
	int row = source.rows;
	int _max = col > row ? col : row;

	cv::Mat resized = cv::Mat::zeros(_max, _max, CV_8UC3);
	source.copyTo(resized(cv::Rect(0, 0, col, row)));


	return resized;
}

void detect(cv::Mat& image, cv::dnn::Net& net, const std::vector<std::string>& class_list, std::vector<Detection>& output)
{
	auto input_image = format_yolov5(image);

	float x_factor = float(input_image.cols) / INPUT_WIDTH;
	float y_factor = float(input_image.rows) / INPUT_HEIGHT;

	cv::Mat blob = cv::dnn::blobFromImage(input_image, 1.0f / 255, cv::Size(INPUT_WIDTH, INPUT_HEIGHT), cv::Scalar(), true, false);

	std::vector<cv::Mat> outputs;
	net.setInput(blob);
	net.forward(outputs, net.getUnconnectedOutLayersNames());

	float* data = (float*)outputs[0].data;

	const int rows = 25200;

	std::vector<int> class_ids;
	std::vector<float> confidences;
	std::vector<cv::Rect> boxes;

	for (int i = 0; i < rows; ++i)
	{
		float confidence = data[4];
		if (confidence > CONFIDENCE_THRESHOLD)
		{
			float* class_scores = data + 5;
			cv::Mat scores(1, class_list.size(), CV_32FC1, class_scores);
			cv::Point class_id;
			double max_class_score;
			cv::minMaxLoc(scores, 0, &max_class_score, 0, &class_id);
			if (max_class_score > SCORE_THRESHOLD)
			{
				confidences.push_back(confidence);
				class_ids.push_back(class_id.x);

				float x = data[0];
				float y = data[1];
				float w = data[2];
				float h = data[3];

				int left = int((x - 0.5 * w) * x_factor);
				int top = int((y - 0.5 * h) * y_factor);
				int width = int(w * x_factor);
				int height = int(h * y_factor);
				boxes.push_back(cv::Rect(left, top, width, height));
			}
		}
		data += DEMENSIONS;
	}

	std::vector<int> nms_result;
	cv::dnn::NMSBoxes(boxes, confidences, SCORE_THRESHOLD, NMS_THRESHOLD, nms_result);
	for (int i = 0; i < nms_result.size(); ++i)
	{
		int idx = nms_result[i];
		Detection result;
		result.class_id = class_ids[idx];
		result.confidence = confidences[idx];
		result.box = boxes[idx];

		output.push_back(result);
	}
}

int signal_quality_eval(const char* modelName, const char* imgFileName, const float conf_thresh)
{
	// 字符串转换
	std::string modelName_s = modelName;
	std::string imgFileName_s = imgFileName;

	// 定义标签
	std::vector<std::string> class_list;
	class_list.push_back("QRS");
	class_list.push_back("T");

	// 加载模型
	cv::dnn::Net net;
	bool is_cuda = 0;
	load_net(modelName_s, is_cuda, net);

	std::vector<cv::String> files;
	cv::glob(imgFileName_s, files);

	int file_num = files.size();

	// 如果图像数量不满36张，返回-1
	if (file_num < 36)
	{
		return -1;
	}

	int count = 0;
	float confidence_sum = 0.0f;
	for (int i = 0; i < file_num; ++i)
	{
		std::vector<Detection> output;
		cv::Mat img = cv::imread(files[i]);
		detect(img, net, class_list, output);

		int detections = output.size();
		for (int j = 0; j < detections; ++j)
		{
			auto detection = output[j];
			auto classID = detection.class_id;
			auto confidence = detection.confidence;

			if (classID == 0)
			{
				confidence_sum += confidence;
			}
		}
	}

	int result = confidence_sum / file_num * 100;
	return result;
}


/*
int main(int argc,char **argv)
{
	// 统计耗时
	LARGE_INTEGER freq, begin, end;

	// 读取类别文件
	std::string className = "./classes/classes.txt";
	std::vector<std::string> class_list = load_class_list(className);

	// 加载模型
	std::string modelName = "./model/best.onnx";
	bool is_cuda = 0;
	cv::dnn::Net net;
	load_net(modelName, is_cuda, net);

	// 读取图像
	cv::Mat img = cv::imread("./data/21.png");

	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&begin);

	// YOLO目标检测
	std::vector<Detection> output;
	detect(img, net, class_list, output);

	QueryPerformanceCounter(&end);
	double costTime = (end.QuadPart - begin.QuadPart) * 1000.0f / freq.QuadPart;
	std::cout << "Detect process cost time:" << costTime << std::endl;

	// 绘制检测结果
	int detections = output.size();
	for (int i = 0; i < detections; ++i)
	{
		auto detection = output[i];
		auto box = detection.box;
		auto classID = detection.class_id;
		auto confidence = detection.confidence;
		auto color = colors[classID % colors.size()];
		cv::rectangle(img, box, color, 3);

		cv::rectangle(img, cv::Point(box.x, box.y - 20), cv::Point(box.x + box.width, box.y), color, cv::FILLED);
		cv::putText(img, class_list[classID].c_str(), cv::Point(box.x, box.y - 5), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 0), 2);
		std::string conf = std::to_string(confidence);
		cv::putText(img, conf.c_str(), cv::Point(box.x + 100, box.y - 5), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 0), 2);
	}

	cv::imshow("detect_Result", img);
	cv::waitKey(0);

	return 0;
}
*/



int getVersion(char* version, int len)
{
	const char* ver = "1.0.0.2";
	return strcpy_s(version, len, ver);
}