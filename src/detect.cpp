#include <fstream>
#include <sstream>
#include <windows.h>
#include <opencv2/opencv.hpp>
#include "detect.h"
#include "evaluate.h"
#include <io.h>
#include <stdio.h>
#include <numeric>

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
		//std::cout << "use CUDA\n";
		result.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
		result.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA_FP16);
	}
	else
	{
		//std::cout << "Runing on CPU\n";
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

void changeFilesOrder(std::vector<cv::String>& files)
{
	std::vector<int> num;
	for (int i = 0; i < files.size(); i++)
	{
		std::string file_s = files[i].c_str();
		size_t pos = file_s.find_last_of("\\");
		std::string name = file_s.substr(pos + 1, file_s.size());
		num.push_back(std::atoi(name.c_str()));
	}
	std::vector<int> idx(num.size());
	iota(idx.begin(), idx.end(), 0);
	sort(idx.begin(), idx.end(), [&num](int i1, int i2) {return num[i1] < num[i2]; });
	
	std::vector<cv::String> filesNew;
	for (int i = 0; i < files.size(); i++)
	{
		filesNew.push_back(files[idx[i]]);
	}
	files.assign(filesNew.begin(), filesNew.end());
}

int checkEdgeInterfere(int *clutter)
{
	int i = 0;
	std::vector<int> top;
	std::vector<int> bottom;
	std::vector<int> left;
	std::vector<int> right;

	for (i = 0; i < 6; i++)
	{
		top.push_back(clutter[i]);
		bottom.push_back(clutter[30+i]);
		left.push_back(clutter[i * 6]);
		right.push_back(clutter[5 + i * 6]);
	}

	int num[4];
	num[0] = std::accumulate(top.begin(), top.end(), 0);
	num[1] = std::accumulate(bottom.begin(), bottom.end(), 0);
	num[2]= std::accumulate(left.begin(), left.end(), 0);
	num[3]= std::accumulate(right.begin(), right.end(), 0);
	for (i = 0; i < 4; i++)
	{
		if (num[i] >= 4)
		{
			return 1;
		}
	}

	return 0;
}

float signal_quality_eval(const char* modelName, const char* imgFileName, const char* BFDfileName, const char* PKfileName, const float thresh)
{
	// 字符串转换
	std::string modelName_s = modelName;
	std::string imgFileName_s = imgFileName;
	std::string BFDfileName_s = BFDfileName;
	std::string PKfileName_s = PKfileName;

	// 定义标签
	std::vector<std::string> class_list;
	class_list.push_back("CLUTTER");
	class_list.push_back("T");
	class_list.push_back("QRS");

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

	changeFilesOrder(files);

	// 记录符合幅值要求的边缘和内部通道
	std::vector<int> channelIndex;
	std::vector<int> channelFlag;
	std::vector<int> interChannels;
	std::vector<int> edgeChannels;
	int extremeFlag = 0;
	int flag = preProcessing(BFDfileName_s, channelIndex, channelFlag, interChannels, edgeChannels, &extremeFlag);
	if (flag < 0)
	{
		return -1;
	}

#if OPENCV_DEBUG
	for (int i = 0; i < file_num; ++i)
	{
		std::string savePath = "F:\\signal_quality_evaluation_3\\temp\\";
		cv::Mat img = cv::imread(files[i]);
		std::string file_s = files[i].c_str();
		size_t pos = file_s.find_last_of("\\");
		std::string name = file_s.substr(pos + 1, file_s.size());
		cv::imwrite(savePath.append(name), img);
	}
#endif

	// 首先，对符合幅值要求的中心区域的通道波形图进行检测
	float clutterNumInter = 0.0f;
	int QRSnum = 0;
	int clutter[36] = { 0 };
	int qrs[36] = { 0 };
	for (int i = 0; i < interChannels.size(); ++i)
	{
		std::vector<Detection> output;
		cv::Mat img = cv::imread(files[interChannels[i]]);
		detect(img, net, class_list, output);

		int detections = output.size();

#if OPENCV_DEBUG
		std::string savePath = "F:\\signal_quality_evaluation_3\\temp\\";
		for (int j = 0; j < detections; ++j)
		{
			auto detection = output[j];
			auto box = detection.box;
			auto classID = detection.class_id;
			auto confidence = detection.confidence;
			auto color = colors[classID % colors.size()];
			cv::rectangle(img, box, color, 3);

			cv::rectangle(img, cv::Point(box.x, box.y - 20), cv::Point(box.x + box.width, box.y), color, cv::FILLED);
			cv::putText(img, class_list[classID].c_str(), cv::Point(box.x, box.y - 5), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 0), 2);
			std::string conf = std::to_string(confidence);
			cv::putText(img, conf.c_str(), cv::Point(box.x + 150, box.y - 5), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 0), 2);
		}
		cv::imwrite(savePath.append(std::to_string(interChannels[i]+1)).append(".png"), img);
#endif

		// 检测波形是否有缺陷
		for (int j = 0; j < detections; ++j)
		{
			auto detection = output[j];
			auto classID = detection.class_id;
			auto confidence = detection.confidence;

			if (classID == 0)
			{
				clutter[interChannels[i]] = 1;
				clutterNumInter++;
				break;
			}
		}

		// 检测是否有QRS波
		for (int j = 0; j < detections; ++j)
		{
			auto detection = output[j];
			auto classID = detection.class_id;
			auto confidence = detection.confidence;

			if (classID == 2)
			{
				qrs[interChannels[i]] = 1;
				QRSnum++;
				break;
			}
		}
	}

	// 然后，对符合幅值要求的边缘区域的通道波形图进行检测
	float clutterNumEdge = 0.0f;
	for (int i = 0; i < edgeChannels.size(); ++i)
	{
		std::vector<Detection> output;
		cv::Mat img = cv::imread(files[edgeChannels[i]]);
		detect(img, net, class_list, output);

		int detections = output.size();

#if OPENCV_DEBUG
		std::string savePath = "F:\\signal_quality_evaluation_3\\temp\\";
		for (int j = 0; j < detections; ++j)
		{
			auto detection = output[j];
			auto box = detection.box;
			auto classID = detection.class_id;
			auto confidence = detection.confidence;
			auto color = colors[classID % colors.size()];
			cv::rectangle(img, box, color, 3);

			cv::rectangle(img, cv::Point(box.x, box.y - 20), cv::Point(box.x + box.width, box.y), color, cv::FILLED);
			cv::putText(img, class_list[classID].c_str(), cv::Point(box.x, box.y - 5), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 0), 2);
			std::string conf = std::to_string(confidence);
			cv::putText(img, conf.c_str(), cv::Point(box.x + 150, box.y - 5), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 0), 2);
		}
		cv::imwrite(savePath.append(std::to_string(edgeChannels[i]+1)).append(".png"), img);
#endif

		// 检测波形是否有缺陷
		for (int j = 0; j < detections; ++j)
		{
			auto detection = output[j];
			auto classID = detection.class_id;
			auto confidence = detection.confidence;

			if (classID == 0)
			{
				clutter[edgeChannels[i]] = 1;
				clutterNumEdge++;
				break;
			}
		}

		// 检测是否有QRS波
		for (int j = 0; j < detections; ++j)
		{
			auto detection = output[j];
			auto classID = detection.class_id;
			auto confidence = detection.confidence;

			if (classID == 2)
			{
				qrs[edgeChannels[i]] = 1;
				QRSnum++;
				break;
			}
		}
	}

	// 边缘通道中幅值排序在前16位的，如果检测到缺陷，每个减5分
	int extra = 0;
	for (int i = 0; i < 16; i++)
	{
		if (channelFlag[i] == 0 && clutter[channelIndex[i]] == 1)
		{
			extra += 5;
		}
	}

	int result = (interChannels.size() - clutterNumInter) * 5 + (edgeChannels.size() - clutterNumEdge) - extra;

	// 针对所有通道都有很大干扰的情况
	if (QRSnum < 3)
	{
		result = 0;
	}

	// 针对边缘某一排或某一列多个通道有干扰的情况，怀疑有金属干扰
	int edgeFlag = checkEdgeInterfere(clutter);
	if (edgeFlag)
	{
		result = result / 2;
	}

	if (result < 0)
	{
		result = 0;
	}

	return result;

	//// 如果深度学习得分低于75，则直接输出得分；否则，再执行evaluate()
	//if (result < thresh * 100)
	//{
	//	return result;
	//}
	//else
	//{
	//	float resultEvaluate = evaluate(BFDfileName_s, PKfileName_s);
	//	return resultEvaluate;
	//}

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
	const char* ver = "2.0.0.1";
	return strcpy_s(version, len, ver);
}