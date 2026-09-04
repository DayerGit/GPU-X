#include "NVIDIA GPU.h"

NVIDIA_GPU::NVIDIA_GPU(IDXGIAdapter* pDXGIAdapter) : GPU(pDXGIAdapter){
	this->_whoIsMyDaddy = TypeOfGPU::NVIDIA_GPU;

}

NVIDIA_GPU::~NVIDIA_GPU() {}