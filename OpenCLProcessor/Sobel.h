#pragma once
#include "BaseProcessor.h"
#include <string>

class Sobel : public BaseProcessor
{
private:
	cl::Buffer	filterHor, filterVer;
	cl::Image2D	inputImg, outputImg;
	//ͼ�������Ϣ
	int			height, width, pitch;
	std::string file;
public:
	Sobel(std::string fileName);
	~Sobel(void);
	//
	virtual void SetArgument();
	virtual void GetArgument();
	void LoadImage2D(std::string& fileName);
	void SaveImage2D(std::string& fileName, unsigned char* buffer);
	//ִ��
	void RunSobel(std::string kernelSource);
};
