// SobelFilter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <atlimage.h>
#include "Sobel.h"
#include "Bilinear.h"
#include "Bilinear_beta.h"
#include "FreeImage.h"
#include <iostream>
using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	//Sobel sobel("G:\\GPU��Ŀ\\������Դ\\jiaoba.jpg");
	//sobel.RunSobel("Sobel");

	//Bilinear bilinear("G:\\GPU��Ŀ\\������Դ\\����ͼ3.bmp");
    //bilinear.RunBilinear("Bilinear");
	
	Bilinear_beta bilinear_beta("G:\\GPU��Ŀ\\������Դ\\����ͼ3.bmp");
	bilinear_beta.RunBilinear("Bilinear_beta");
	getchar();
	return 0;
}

