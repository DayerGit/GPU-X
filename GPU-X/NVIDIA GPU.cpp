#include "NVIDIA GPU.h"

NVIDIA_GPU::NVIDIA_GPU(IDXGIAdapter* pDXGIAdapter, int index) : GPU(pDXGIAdapter, index){
	this->_whoIsMyDaddy = TypeOfGPU::NVIDIA_GPU;

}

NVIDIA_GPU::~NVIDIA_GPU() {}