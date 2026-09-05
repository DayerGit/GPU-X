#include "GPUFactory.h"

GPUFactory::GPUFactory() {
	CreateDXGIFactory(IID_PPV_ARGS(&this->_pDXGIFactory));

	HMODULE hVulkanLib = LoadLibraryA("vulkan-1.dll");
	if (hVulkanLib) {
		this->_pfnCreateInstanceProcAddr = (PFN_vkCreateInstance)GetProcAddress(hVulkanLib, "vkCreateInstance");
		this->_pfnDestroyInstanceProcAddr = (PFN_vkDestroyInstance)GetProcAddress(hVulkanLib, "vkDestroyInstance");

		if (this->_pfnCreateInstanceProcAddr && this->_pfnDestroyInstanceProcAddr) {
			VkApplicationInfo appInfo = {};
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pApplicationName = "GPU-X";
			appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.pEngineName = "No Engine";
			appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.apiVersion = VK_API_VERSION_1_0;

			const char* instanceExtensions[] = {
				"VK_KHR_get_physical_device_properties2"
			};

			VkInstanceCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			createInfo.pApplicationInfo = &appInfo;
			createInfo.enabledExtensionCount = 1;
			createInfo.ppEnabledExtensionNames = instanceExtensions;

			this->_pfnCreateInstanceProcAddr(&createInfo, nullptr, &this->_vkInstance);
		}
	}
}

std::vector<std::unique_ptr<GPU>> GPUFactory::LetsCreateGPUs() {
	if (!this->_pDXGIFactory) return {};

	std::vector<std::unique_ptr<GPU>> result;

	IDXGIAdapter* pAdapter = nullptr;

	for (int i = 0; this->_pDXGIFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
		DXGI_ADAPTER_DESC adapterDesc;
		pAdapter->GetDesc(&adapterDesc);

		switch (adapterDesc.VendorId) {
		case NVIDIA_VENDOR_ID: {
			result.emplace_back(std::make_unique<NVIDIA_GPU>(pAdapter, adapterDesc.AdapterLuid, i, this->_vkInstance));
			break;
		}
		case AMD_VENDOR_ID: {
			result.emplace_back(std::make_unique<AMD_GPU>(pAdapter, adapterDesc.AdapterLuid, i, this->_vkInstance));
			break;
		}
		case INTEL_VENDOR_ID: {
			result.emplace_back(std::make_unique<Intel_GPU>(pAdapter, adapterDesc.AdapterLuid, i, this->_vkInstance));
			break;
		}
		default: {
			result.emplace_back(std::make_unique<GPU>(pAdapter, adapterDesc.AdapterLuid, i, this->_vkInstance));
			break;
		}
		}
	}

	return result;
}

GPUFactory::~GPUFactory() {
	if (this->_pDXGIFactory)
		this->_pDXGIFactory->Release();

	if (this->_vkInstance != VK_NULL_HANDLE && this->_pfnDestroyInstanceProcAddr)
		_pfnDestroyInstanceProcAddr(this->_vkInstance, nullptr);

}