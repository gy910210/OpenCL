#pragma once
#include <cl/cl.hpp>
#include <vector>
#include <string>


class BaseProcessor
{

public:
	std::vector<cl::Platform>	platforms;
	std::vector<cl::Device>		devices;
	cl::CommandQueue			commandQueue;
	cl::Context					context;
	cl::Kernel					kernel;
	cl::Program					program;
	cl_int						errorNum;

private:
	std::string LoadKernelSourceFromFile(std::string, cl_int&);

public:
	BaseProcessor();
	~BaseProcessor();
	BOOL CheckError(cl_int error, const char* stepDesp);
	void LoadProgram(std::string);
	void CreateKernel(const std::string);
	void RunKernel(cl::NDRange&, cl::NDRange&, cl::NDRange&);

public:
	//�������ݣ����麯��������ʵ��
	virtual void GetArgument() = 0;
	virtual void SetArgument() = 0;
};

