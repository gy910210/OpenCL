#include "StdAfx.h"
#include "Sobel.h"
#include <string>
#include <iostream>
#include "FreeImage.h"
using namespace std;


Sobel::Sobel(std::string fileName)
{
	int filterHorArray[9] = {-1, -2, -1, 0,	 0,	0,	1,	2,	1};
	int filterVerArray[9] = {-1, 0,	 1,	 -2, 0,	2,	-1,	0,	1};
	//����ˮƽ�ʹ�ֱ�Ĺ�����buffer
	filterHor = cl::Buffer::Buffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, 9*sizeof(int), filterHorArray, &errorNum);
	CheckError(errorNum, "Allocation of Filter Horizontal...");
	filterVer = cl::Buffer::Buffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, 9*sizeof(int), filterVerArray, &errorNum);
	CheckError(errorNum, "Allocation of Filter Vertical...");
	//�����������ͼ��
	file = fileName;
	LoadImage2D(fileName);
}


Sobel::~Sobel(void)
{
}

void Sobel::SetArgument()
{
	errorNum =  kernel.setArg(0, inputImg);
	errorNum |= kernel.setArg(1, outputImg);
	errorNum |= kernel.setArg(2, filterHor);
	errorNum |= kernel.setArg(3, filterVer);
	//errorNum |= kernel.setArg(4, sizeof(cl_int), &width);
	//errorNum |= kernel.setArg(5, sizeof(cl_int), &height);
}

void Sobel::GetArgument()
{
	//buffer
	unsigned char* buffer = new unsigned char[width*height*4];
	//��ȡoutputͼ�񲢱��浽������buffer
	cl::size_t<3> origin, region;
	origin[0] = 0;
	origin[1] = 0;
	origin[2] = 0;
	region[0] = width;
	region[1] = height;
	region[2] = 1;
	//ִ�ж�ͼ�������̰߳�ȫ��
	errorNum = commandQueue.enqueueReadImage(outputImg, CL_TRUE, origin, region, 0, 0, buffer);
	CheckError(errorNum, "Reading Image From Devices...");
	//��������
	commandQueue.finish();
	SaveImage2D(file.append("outout.bmp"), buffer);
}

void Sobel::LoadImage2D(std::string& fileName)
{
	width = 0;
	height = 0;
	pitch = 0;
	//��freeimage ��ȡ�����ͼ��
	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(fileName.c_str(), 0);
	FIBITMAP* image = FreeImage_Load(format, fileName.c_str());
	//��ʱͼ��תΪ32bits
	FIBITMAP* temp = image;
	image = FreeImage_ConvertTo32Bits(image);
	FreeImage_Unload(temp);
	//��ó���
	width = FreeImage_GetWidth(image);
	height = FreeImage_GetHeight(image);
	pitch = FreeImage_GetPitch(image);
	unsigned char* buffer = new unsigned char[width*height*4];
	//����ͼ��λ��Ϣ
	memcpy(buffer, FreeImage_GetBits(image), width*height*4);
	//�ͷ�ͼ��
	FreeImage_Unload(image);
	//////////////////////////////////////////////////////////////////////////
	//����image2d,����ͼ��
	cl::ImageFormat clFormat(CL_RGBA, CL_UNSIGNED_INT8);
	//flags
	cl_mem_flags flags = CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR;
	inputImg = cl::Image2D(context, CL_MEM_COPY_HOST_PTR, clFormat, width, height, 0, (void*)buffer, &errorNum);
	delete[] buffer;
	std::string strMess = "Loading Input Image From [" + fileName + "]...";
	CheckError(errorNum, strMess.data());
	//�������ͼ��
	cl::ImageFormat clFormat2(CL_RGBA, CL_UNSIGNED_INT8);
	outputImg = cl::Image2D(context, CL_MEM_WRITE_ONLY, clFormat2, width, height, 0, NULL, &errorNum);
	CheckError(errorNum, "Creating Output Image...");
}

void Sobel::SaveImage2D(std::string& fileName, unsigned char* buffer)
{
	//ʹ��freeimage���������ͼ���ļ�
	FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilename(fileName.data());
	FIBITMAP *image = FreeImage_ConvertFromRawBits((BYTE*)buffer, width, height, width*4, 32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);
	image = FreeImage_ConvertTo24Bits(image);
	FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(image);
	WORD bpp = FreeImage_GetBPP(image);
	BOOL saved = (FreeImage_FIFSupportsWriting(format) && FreeImage_FIFSupportsExportBPP(format, bpp));
	//����
	BOOL result = FreeImage_Save(format ,image, fileName.data(), BMP_DEFAULT);
}

//input "Sobel"
void Sobel::RunSobel(std::string kernelSource)
{
	LoadProgram(kernelSource);
	CreateKernel(kernelSource);
	cl::NDRange nullRange = cl::NullRange;
	RunKernel(nullRange, cl::NDRange(width, height), nullRange);
	GetArgument();
}