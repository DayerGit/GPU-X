#include "AMD GPU.h"

AMD_GPU::AMD_GPU(IDXGIAdapter* pDXGIAdapter, int index) : GPU(pDXGIAdapter, index) {
	this->_whoIsMyDaddy = TypeOfGPU::AMD_GPU;

}

AMD_GPU::~AMD_GPU() {}