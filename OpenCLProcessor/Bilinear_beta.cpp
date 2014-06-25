#include "StdAfx.h"
#include "Bilinear_beta.h"
#include <string>
#include <iostream>
#include "FreeImage.h"
#include <CL/cl.hpp>
#include "clTimer.h"
#include <cstdio>
using namespace std;

int roundUp(int values, int mutiple)
{
	int remainder=values%mutiple;
	if(remainder!=0)
	{
		values+=(mutiple-remainder);
	}
	return values;
}

Bilinear_beta::Bilinear_beta(std::string fileName)
{
	//�����������ͼ��
	size=0.5;
	file = fileName;
	pSrc=NULL;
	pDes=NULL;
	LoadImage2D(fileName);
}

Bilinear_beta::~Bilinear_beta( )
{
	FreeImage_Unload(srcImage);
	FreeImage_Unload(desImage);
	delete[] pDes;
}

void Bilinear_beta::SetArgument( )
{
	errorNum =  kernel.setArg(0,inputImg);
	errorNum |= kernel.setArg(1, width);
	errorNum |= kernel.setArg(2,height);
	errorNum |= kernel.setArg(3,outputImg);
	errorNum |= kernel.setArg(4,t_width);
	errorNum |= kernel.setArg(5,t_height);
	CheckError(errorNum, "Setting arguments...");
}

void Bilinear_beta::GetArgument( )
{
	
	//ִ�ж�ͼ�������̰߳�ȫ��
	errorNum = commandQueue.enqueueReadBuffer(outputImg,CL_TRUE, 0, t_width*t_height*sizeof(BYTE),(void *)pDes);
	CheckError(errorNum, "Reading Image From Devices...");
	//��������
	commandQueue.finish();
	SaveImage2D(file.append("outout.bmp"));
}

void Bilinear_beta::LoadImage2D(std::string& fileName)
{
	width = 0;
	height = 0;
	pitch = 0;
	//��freeimage ��ȡ�����ͼ��
	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(fileName.c_str(), 0);
	srcImage = FreeImage_Load(format, fileName.c_str());

	width = FreeImage_GetWidth(srcImage);
	height = FreeImage_GetHeight(srcImage);
	pitch = FreeImage_GetPitch(srcImage);
	pSrc	= FreeImage_GetBits(srcImage);

	//cout<<width<<" "<<height<<" "<<pitch<<endl;
	//cout<<FreeImage_GetBPP(srcImage)<<endl;
	//cout<<FreeImage_GetLine(srcImage)/ width<<endl;

	inputImg=cl::Buffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, sizeof(BYTE)*width*height,pSrc,&errorNum);
	std::string strMess = "Loading Input Image From [" + fileName + "]...";
	CheckError(errorNum, strMess.data());

	//�������ͼ��
	t_width=(int)(size*width);
	t_height=(int)(size*height);

	t_width=roundUp(t_width,16);
	t_height=roundUp(t_height,16);

	pDes=new BYTE[t_height*t_width];
	outputImg=cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(BYTE)*t_width*t_height, NULL, &errorNum);
	CheckError(errorNum, "Creating Output Image...");
}

void Bilinear_beta::SaveImage2D(std::string& fileName)
{
	//ʹ��freeimage���������ͼ���ļ�
	FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilename(fileName.data());
	desImage=FreeImage_ConvertFromRawBits((BYTE *)pDes, t_width, t_height, t_width,8,FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);

	FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(desImage);
	WORD bpp = FreeImage_GetBPP(desImage);
	BOOL saved = (FreeImage_FIFSupportsWriting(format) && FreeImage_FIFSupportsExportBPP(format, bpp));
	//����
	BOOL result = FreeImage_Save(format ,desImage, fileName.data(), BMP_DEFAULT);
}

//input "Bilinear"
void Bilinear_beta::RunBilinear(std::string kernelSource)
{
	LoadProgram(kernelSource);
	CreateKernel(kernelSource);

	cl::NDRange nullRange = cl::NullRange;
	cl::NDRange globalws(t_width,t_height);
	cl::NDRange localws(16, 16);

	clTimer timer;
	timer.Start();
	RunKernel(nullRange, globalws, localws);
	timer.Stop();
	GetArgument();
	printf("kernel total time:%.6f ms\n", timer.GetElapsedTime()*1000);
}