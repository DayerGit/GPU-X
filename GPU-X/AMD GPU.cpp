#include "AMD GPU.h"

AMD_GPU::AMD_GPU(IDXGIAdapter* pDXGIAdapter) : GPU(pDXGIAdapter) {
	this->_whoIsMyDaddy = TypeOfGPU::AMD_GPU;

}

AMD_GPU::~AMD_GPU() {}