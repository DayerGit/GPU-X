#include "AMD GPU.h"

#include <iostream>

AMD_GPU::AMD_GPU(IDXGIAdapter* pDXGIAdapter, LUID AdapterLUID, int index, VkInstance vkInstance)
	: GPU(pDXGIAdapter, AdapterLUID, index, vkInstance) 
{
	this->_whoIsMyDaddy = TypeOfGPU::AMD_GPU;

	if (!this->_LoadLib()) {
		std::cerr << "Error _LoadLib!" << std::endl;
		return;
	}

	if (!this->_GetDeviceHandle()) {
		std::cerr << "Error _GetDeviceHandle!" << std::endl;
		return;
	}

	this->_FillBIOSInfo();
}

void* __stdcall ADL_Main_Memory_Alloc(int iSize) {
	return malloc(iSize);
}

void AMD_GPU::UpdateSensors() {
}

bool AMD_GPU::_LoadLib() {
	std::cout << "Let's Start _LoadLib" << std::endl;

#ifdef _WIN64
	this->_hAModule = LoadLibraryA("atiadlxx.dll");
#else
	this->_hAModule = LoadLibraryA("atiadlxy.dll");
#endif

	if (!this->_hAModule) {
		std::cerr << "Error Loading AMD Library!" << std::endl;
		return false;
	}

	this->_ADL_MAIN_CONTROL_CREATE = (ADL_MAIN_CONTROL_CREATE)GetProcAddress(this->_hAModule, "ADL_Main_Control_Create");
	if (!this->_ADL_MAIN_CONTROL_CREATE) {
		std::cerr << "Error Loading ADL_Main_Control_Create!" << std::endl;
		return false;
	}

	this->_ADL_ADAPTER_NUMBEROFADAPTERS_GET = (ADL_ADAPTER_NUMBEROFADAPTERS_GET)GetProcAddress(this->_hAModule, "ADL_Adapter_NumberOfAdapters_Get");
	if (!this->_ADL_ADAPTER_NUMBEROFADAPTERS_GET) {
		std::cerr << "Error Loading ADL_Adapter_NumberOfAdapters_Get!" << std::endl;
		return false;
	}

	this->_ADL_ADAPTER_ADAPTERINFO_GET = (ADL_ADAPTER_ADAPTERINFO_GET)GetProcAddress(this->_hAModule, "ADL_Adapter_AdapterInfo_Get");
	if (!this->_ADL_ADAPTER_ADAPTERINFO_GET) {
		std::cerr << "Error Loading ADL_Adapter_AdapterInfo_Get!" << std::endl;
		return false;
	}

	this->_ADL_ADAPTER_VIDEOBIOSINFO_GET = (ADL_ADAPTER_VIDEOBIOSINFO_GET)GetProcAddress(this->_hAModule, "ADL_Adapter_VideoBiosInfo_Get");
	if (!this->_ADL_ADAPTER_VIDEOBIOSINFO_GET) {
		std::cerr << "Error Loading ADL_Adapter_VideoBiosInfo_Get!" << std::endl;
		return false;
	}

	this->_ADL_MAIN_CONTROL_DESTROY = (ADL_MAIN_CONTROL_DESTROY)GetProcAddress(this->_hAModule, "ADL_Main_Control_Destroy");
	if (!this->_ADL_MAIN_CONTROL_DESTROY) {
		std::cerr << "Error Loading ADL_Main_Control_Destroy!" << std::endl;
		return false;
	}

	int result = this->_ADL_MAIN_CONTROL_CREATE(ADL_Main_Memory_Alloc, 1);

	if (result != ADL_OK) {
		std::cerr << "Error init ADL! Result " << result << std::endl;
		return false;
	}

	return true;
}

bool AMD_GPU::_GetDeviceHandle() {
	std::cout << "Let's Start _GetDeviceHandle" << std::endl;

	if (!this->_pciLocationValid) {
		std::cerr << "PCI location unknown, cannot match ADL adapter reliably!" << std::endl;
		return false;
	}

	int numAdapters = 0;
	int result = this->_ADL_ADAPTER_NUMBEROFADAPTERS_GET(&numAdapters);

	if (ADL_OK == result && numAdapters > 0) {
		LPAdapterInfo adapterInfo = (LPAdapterInfo)calloc(numAdapters, sizeof(AdapterInfo));

		this->_ADL_ADAPTER_ADAPTERINFO_GET(adapterInfo, sizeof(AdapterInfo) * numAdapters);

		for (int i = 0; i < numAdapters; i++) {
			if (adapterInfo[i].iBusNumber == static_cast<int>(this->_pciBusNumber) &&
				adapterInfo[i].iDeviceNumber == static_cast<int>(this->_pciDeviceNumber) &&
				adapterInfo[i].iFunctionNumber == static_cast<int>(this->_pciFunctionNumber)) {
				this->_physAdapterIndex = adapterInfo[i].iAdapterIndex;
				free(adapterInfo);
				return true;
			}
		}
		free(adapterInfo);
		std::cerr << "No AMD ADL adapter matched by PCI location!" << std::endl;
	}
	else std::cerr << "Not Found AMD GPU! Result " << result << std::endl;

	return false;
}

void AMD_GPU::_FillBIOSInfo() {
	std::cout << "Let's Start _FillBIOSInfo" << std::endl;
	ADLBiosInfo biosInfo;
	int result = this->_ADL_ADAPTER_VIDEOBIOSINFO_GET(this->_physAdapterIndex, &biosInfo);
	if (ADL_OK == result) {
		std::string asciiString(biosInfo.strVersion);
		this->BIOS.version = std::wstring(asciiString.begin(), asciiString.end());
	}
	else std::cerr << "Error _ADL_ADAPTER_VIDEOBIOSINFO_GET! Result " << result << std::endl;
}

AMD_GPU::~AMD_GPU() {
	if (this->_ADL_MAIN_CONTROL_DESTROY)
		this->_ADL_MAIN_CONTROL_DESTROY();
}