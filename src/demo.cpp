#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <opencv2/opencv.hpp>
#include "detect.h"
#include <vector>
#include <io.h>

void getFiles(std::string path, std::vector<std::string>& files)
{
	// 文件句柄
	intptr_t hFile = 0;
	// 文件信息
	struct _finddata_t fileinfo;

	std::string p;

	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1) {
		do {
			// 保存文件的全路径
			files.push_back(p.assign(path).append("\\").append(fileinfo.name));

		} while (_findnext(hFile, &fileinfo) == 0);  //寻找下一个，成功返回0，否则-1

		_findclose(hFile);
	}
}

void getFilesExt(std::string path, std::vector<std::string>& files, std::string ext)
{
	// 文件句柄
	intptr_t hFile = 0;
	// 文件信息
	struct _finddata_t fileinfo;

	std::string p;

	if ((hFile = _findfirst(p.assign(path).append("\\*"+ext).c_str(), &fileinfo)) != -1) {
		do {
			// 保存文件的全路径
			files.push_back(p.assign(path).append("\\").append(fileinfo.name));

		} while (_findnext(hFile, &fileinfo) == 0);  //寻找下一个，成功返回0，否则-1

		_findclose(hFile);
	}
}

int main(int argc, char** argv)
{
	// 获取版本号
	char version[] = "";
	getVersion(version, 8);

	// 统计耗时
	LARGE_INTEGER freq, begin, end;

	// 加载模型
	char* modelName = "./model/best.onnx";

	char* filePath = "D:\\MCGData";        // 输入样本存放路径

	std::vector<std::string> files;

	//获取该路径下的所有文件
	getFiles(filePath, files);

	char str[30];
	int size = files.size();

	for (int i = 51; i < size; i++)
	{
		std::vector<std::string> fileTemp;
		getFiles(files[i], fileTemp);

		std::vector<std::string> BFDfile;
		getFilesExt(fileTemp[2], BFDfile, ".BFD");

		std::vector<std::string> PKfile;
		getFilesExt(fileTemp[2], PKfile, ".PK");

		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&begin);

		std::string a = fileTemp[2].append("\\BFDImg\\*.png");

		std::string file_s = files[i].c_str();
		size_t pos = file_s.find_last_of("\\");
		std::string name = file_s.substr(pos + 1, file_s.size());

		// 质量评价
		const char* imgFileName = a.c_str();
		const char* bfdFileName = BFDfile[0].c_str();
		const char* pkFileName = PKfile[0].c_str();
		const float thresh = 0.7f;
		float result = signal_quality_eval(modelName, imgFileName, bfdFileName, pkFileName, thresh);

		QueryPerformanceCounter(&end);
		double costTime = (end.QuadPart - begin.QuadPart) * 1000.0f / freq.QuadPart;
		//std::cout << "Detect process cost time:" << costTime << std::endl;
		std::cout << name << ":"<< result << std::endl;

	}
	
	return 0;
}


