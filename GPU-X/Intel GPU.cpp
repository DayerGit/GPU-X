#include "Intel GPU.h"

Intel_GPU::Intel_GPU(IDXGIAdapter* pDXGIAdapter, LUID AdapterLUID, int index, VkInstance vkInstance) 
	: GPU(pDXGIAdapter, AdapterLUID, index, vkInstance) 
{
	this->_whoIsMyDaddy = TypeOfGPU::INTEL_GPU;

}

Intel_GPU::~Intel_GPU() {}