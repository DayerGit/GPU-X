#include "NVIDIA GPU.h"

NVIDIA_GPU::NVIDIA_GPU(IDXGIAdapter* pDXGIAdapter, LUID AdapterLUID, int index, VkInstance vkInstance) 
						: GPU(pDXGIAdapter, AdapterLUID, index, vkInstance)
{
	this->_whoIsMyDaddy = TypeOfGPU::NVIDIA_GPU;

}

NVIDIA_GPU::~NVIDIA_GPU() {}