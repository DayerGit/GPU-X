#include "AMD GPU.h"

AMD_GPU::AMD_GPU(IDXGIAdapter* pDXGIAdapter, LUID AdapterLUID, int index, VkInstance vkInstance)
	: GPU(pDXGIAdapter, AdapterLUID, index, vkInstance) 
{
	this->_whoIsMyDaddy = TypeOfGPU::AMD_GPU;

}

AMD_GPU::~AMD_GPU() {}