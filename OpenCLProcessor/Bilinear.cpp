#include "StdAfx.h"
#include "Bilinear.h"
#include <string>
#include <iostream>
#include "FreeImage.h"
#include <CL/cl.hpp>
#include "clTimer.h"
using namespace std;

Bilinear::Bilinear(std::string fileName)
{
	//�����������ͼ��
	size=0.5;
	file = fileName;
	LoadImage2D(fileName);
}

Bilinear::~Bilinear( )
{
	FreeImage_Unload(srcImage);
	FreeImage_Unload(desImage);
}

void Bilinear::SetArgument( )
{
	errorNum =  kernel.setArg(0,inputImg);
	errorNum |= kernel.setArg(1,outputImg);
	errorNum |= kernel.setArg(2,size);
}

void Bilinear::GetArgument( )
{
	//buffer
	unsigned char* buffer = new unsigned char[t_width*t_height];
	//��ȡoutputͼ�񲢱��浽������buffer
	cl::size_t<3> origin, region;
	origin[0] = 0;
	origin[1] = 0;
	origin[2] = 0;
	region[0] = t_width;
	region[1] = t_height;
	region[2] = 1;
	//ִ�ж�ͼ�������̰߳�ȫ��
	errorNum = commandQueue.enqueueReadImage(outputImg, CL_TRUE, origin, region, 0, 0, buffer,NULL,NULL);
	CheckError(errorNum, "Reading Image From Devices...");
	//��������
	commandQueue.finish();
	SaveImage2D(file.append("outout.bmp"), buffer);
}

void Bilinear::LoadImage2D(std::string& fileName)
{
	width = 0;
	height = 0;
	pitch = 0;
	//��freeimage ��ȡ�����ͼ��
	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(fileName.c_str(), 0);
	srcImage = FreeImage_Load(format, fileName.c_str());

	//��ó���
	width = FreeImage_GetWidth(srcImage);
	height = FreeImage_GetHeight(srcImage);
	pitch = FreeImage_GetPitch(srcImage);

	//cout<<width<<" "<<height<<" "<<pitch<<endl;
	//cout<<FreeImage_GetBPP(srcImage)<<endl;
    //cout<<FreeImage_GetLine(srcImage)/ width<<endl;
	
	//����ͼ��λ��Ϣ
	unsigned char* buffer = new unsigned char[width*height];
	memcpy(buffer, FreeImage_GetBits(srcImage), width*height);
	
	//����image2d,����ͼ��
	cl::ImageFormat clFormat(CL_R, CL_UNSIGNED_INT8);
	//flags
	cl_mem_flags flags = CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR;
	inputImg = cl::Image2D(context, flags, clFormat, width, height, 0, (void*)buffer, &errorNum);
	delete[] buffer;
	std::string strMess = "Loading Input Image From [" + fileName + "]...";
	CheckError(errorNum, strMess.data());

	//�������ͼ��
	t_width=(int)(size*width);
	t_height=(int)(size*height);

	cl::ImageFormat clFormat2(CL_R, CL_UNSIGNED_INT8);
	outputImg = cl::Image2D(context, CL_MEM_WRITE_ONLY, clFormat2, t_width, t_height, 0, NULL, &errorNum);
	CheckError(errorNum, "Creating Output Image...");
}

void Bilinear::SaveImage2D(std::string& fileName, unsigned char* buffer)
{
	//ʹ��freeimage���������ͼ���ļ�
	FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilename(fileName.data());
	desImage=FreeImage_ConvertFromRawBits((BYTE *)buffer, t_width, t_height, t_width,8,FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);
	delete[] buffer;

	FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(desImage);
	WORD bpp = FreeImage_GetBPP(desImage);
	BOOL saved = (FreeImage_FIFSupportsWriting(format) && FreeImage_FIFSupportsExportBPP(format, bpp));
	//����
	BOOL result = FreeImage_Save(format ,desImage, fileName.data(), BMP_DEFAULT);
}

//input "Bilinear"
void Bilinear::RunBilinear(std::string kernelSource)
{

	LoadProgram(kernelSource);
	CreateKernel(kernelSource);

	cl::NDRange nullRange = cl::NullRange;
	cl::NDRange globalws(t_width,t_height);
	//cl::NDRange localws(16, 16);

	clTimer timer;
	timer.Start();
	RunKernel(nullRange, globalws, nullRange);
	timer.Stop();
	GetArgument();
	printf("kernel total time:%.6f ms\n", timer.GetElapsedTime()*1000);
}