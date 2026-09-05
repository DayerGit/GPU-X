#include "Intel GPU.h"

Intel_GPU::Intel_GPU(IDXGIAdapter* pDXGIAdapter, int index) : GPU(pDXGIAdapter, index) {
	this->_whoIsMyDaddy = TypeOfGPU::INTEL_GPU;

}

Intel_GPU::~Intel_GPU() {}