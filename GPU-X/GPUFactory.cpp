#include "GPUFactory.h"

GPUFactory::GPUFactory() {
	CreateDXGIFactory(IID_PPV_ARGS(&this->_pDXGIFactory));
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
			result.emplace_back(std::make_unique<NVIDIA_GPU>(pAdapter));
			break;
		}
		case AMD_VENDOR_ID: {
			result.emplace_back(std::make_unique<AMD_GPU>(pAdapter));
			break;
		}
		case INTEL_VENDOR_ID: {
			result.emplace_back(std::make_unique<Intel_GPU>(pAdapter));
			break;
		}
		default: {
			result.emplace_back(std::make_unique<GPU>(pAdapter));
			break;
		}
		}
	}

	return result;
}

GPUFactory::~GPUFactory() {
	if (this->_pDXGIFactory)
		this->_pDXGIFactory->Release();
}