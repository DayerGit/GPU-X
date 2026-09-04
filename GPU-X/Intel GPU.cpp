#include "Intel GPU.h"

Intel_GPU::Intel_GPU(IDXGIAdapter* pDXGIAdapter) : GPU(pDXGIAdapter) {
	this->_whoIsMyDaddy = TypeOfGPU::INTEL_GPU;

}

Intel_GPU::~Intel_GPU() {}