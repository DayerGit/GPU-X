#include "AMD GPU.h"

#include <iostream>

#include <pciprop.h>

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
	this->_FillBusInfo();
	this->_FillMemoryInfo();
}

void* __stdcall ADL_Main_Memory_Alloc(int iSize) {
	return malloc(iSize);
}

void AMD_GPU::UpdateSensors() {
}

bool AMD_GPU::_LoadLib() {

#ifdef _WIN64
	this->_hAModule = LoadLibraryA("atiadlxx.dll");
#else
	this->_hAModule = LoadLibraryA("atiadlxy.dll");
#endif

	if (!this->_hAModule) return false;

	this->_ADL_MAIN_CONTROL_CREATE = (ADL_MAIN_CONTROL_CREATE)GetProcAddress(this->_hAModule, "ADL_Main_Control_Create");
	if (!this->_ADL_MAIN_CONTROL_CREATE) return false;

	this->_ADL_ADAPTER_NUMBEROFADAPTERS_GET = (ADL_ADAPTER_NUMBEROFADAPTERS_GET)GetProcAddress(this->_hAModule, "ADL_Adapter_NumberOfAdapters_Get");
	if (!this->_ADL_ADAPTER_NUMBEROFADAPTERS_GET) return false;

	this->_ADL_ADAPTER_ADAPTERINFO_GET = (ADL_ADAPTER_ADAPTERINFO_GET)GetProcAddress(this->_hAModule, "ADL_Adapter_AdapterInfo_Get");
	if (!this->_ADL_ADAPTER_ADAPTERINFO_GET) return false;

	this->_ADL_ADAPTER_VIDEOBIOSINFO_GET = (ADL_ADAPTER_VIDEOBIOSINFO_GET)GetProcAddress(this->_hAModule, "ADL_Adapter_VideoBiosInfo_Get");
	if (!this->_ADL_ADAPTER_VIDEOBIOSINFO_GET) return false;

	this->_ADL_ADAPTER_MEMORYINFO_GET = (ADL_ADAPTER_MEMORYINFO_GET)GetProcAddress(this->_hAModule, "ADL_Adapter_MemoryInfo_Get");
	if (!this->_ADL_ADAPTER_MEMORYINFO_GET) {
		std::cerr << "Can't Load ADL_Adapter_MemoryInfo_Get!" << std::endl;
		return false;
	}

	this->_ADL_MAIN_CONTROL_DESTROY = (ADL_MAIN_CONTROL_DESTROY)GetProcAddress(this->_hAModule, "ADL_Main_Control_Destroy");
	if (!this->_ADL_MAIN_CONTROL_DESTROY) return false;

	int result = this->_ADL_MAIN_CONTROL_CREATE(ADL_Main_Memory_Alloc, 1);

	if (result != ADL_OK) return false;

	return true;
}

bool AMD_GPU::_GetDeviceHandle() {
	if (!this->_pciLocationValid) return false;

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
	}

	return false;
}

void AMD_GPU::_FillMemoryInfo() {
	ADLMemoryInfo adlMem;
	int result = this->_ADL_ADAPTER_MEMORYINFO_GET(this->_physAdapterIndex, &adlMem);
	if (ADL_OK == result) {
		std::string asciiString(adlMem.strMemoryType);
		this->Memory.memoryType = std::wstring(asciiString.begin(), asciiString.end());
	}
	else this->Memory.memoryType = L"UNKNOWN";
}

std::wstring AMD_GPU::_FormatPcieString(ULONG speed, ULONG width) {
	if (speed == 0 || width == 0) return L"UNKNOWN";

	const wchar_t* gen = L"?";
	switch (speed) {
	case 1: gen = L"1.0"; break;  
	case 2: gen = L"2.0"; break;  
	case 3: gen = L"3.0"; break;  
	case 4: gen = L"4.0"; break;  
	case 5: gen = L"5.0"; break;  
	case 6: gen = L"6.0"; break;  
	default: gen = L"?"; break;
	}

	return L"PCIe " + std::wstring(gen) + L" x" + std::to_wstring(width);
}

void AMD_GPU::_FillBusInfo() {
	DEVPROPTYPE propType = 0;
	DWORD requiredSize = 0;

	ULONG currentSpeed = 0;
	ULONG currentWidth = 0;
	ULONG maxSpeed = 0;
	ULONG maxWidth = 0;

	SetupDiGetDevicePropertyW(this->_hDevInfo, &this->_devInfoData, &DEVPKEY_PciDevice_CurrentLinkSpeed, &propType, reinterpret_cast<PBYTE>(&currentSpeed), sizeof(currentSpeed), &requiredSize, 0);
	SetupDiGetDevicePropertyW(this->_hDevInfo, &this->_devInfoData, &DEVPKEY_PciDevice_CurrentLinkWidth, &propType, reinterpret_cast<PBYTE>(&currentWidth), sizeof(currentWidth), &requiredSize, 0);
	SetupDiGetDevicePropertyW(this->_hDevInfo, &this->_devInfoData, &DEVPKEY_PciDevice_MaxLinkSpeed, &propType, reinterpret_cast<PBYTE>(&maxSpeed), sizeof(maxSpeed), &requiredSize, 0);
	SetupDiGetDevicePropertyW(this->_hDevInfo, &this->_devInfoData, &DEVPKEY_PciDevice_MaxLinkWidth, &propType, reinterpret_cast<PBYTE>(&maxWidth), sizeof(maxWidth), &requiredSize, 0);

	this->Bus.current = this->_FormatPcieString(currentSpeed, currentWidth);
	this->Bus.maximum = this->_FormatPcieString(maxSpeed, maxWidth);
}

void AMD_GPU::_FillBIOSInfo() {
	ADLBiosInfo biosInfo;
	int result = this->_ADL_ADAPTER_VIDEOBIOSINFO_GET(this->_physAdapterIndex, &biosInfo);
	if (ADL_OK == result) {
		std::string asciiString(biosInfo.strVersion);
		this->BIOS.version = std::wstring(asciiString.begin(), asciiString.end());
	}
}

AMD_GPU::~AMD_GPU() {
	if (this->_ADL_MAIN_CONTROL_DESTROY)
		this->_ADL_MAIN_CONTROL_DESTROY();
}