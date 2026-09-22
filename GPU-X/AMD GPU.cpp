#include "AMD GPU.h"

#include <iostream>
#include <algorithm>

#include <pciprop.h>

void* __stdcall ADL_Main_Memory_Alloc(int iSize) {
	return malloc(iSize);
}

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

	if (!this->_GetCurrentGeneration()) {
		std::cerr << "Error _GetCurrentGeneration!" << std::endl;
		return;
	}

	switch (this->_currentGeneration) {
	case 5: {
		this->_overdrive = std::make_unique<Overdrive5>(this->_hAModule, this->_physAdapterIndex);
		break;
	}
	case 6: {
		this->_overdrive = std::make_unique<Overdrive6>(this->_hAModule, this->_physAdapterIndex);
		break;
	}
	case 7: {
		this->_overdrive = std::make_unique<Overdrive7>(this->_hAModule, this->_physAdapterIndex);
		break;
	}
	case 8: {
		this->_overdrive = std::make_unique<Overdrive8>(this->_hAModule, this->_physAdapterIndex);
		break;
	}
	}

	if (!this->_overdrive->LoadLib()) {
		std::cerr << "Error Overdrive class Load!" << std::endl;
		return;
	}

	this->_FillBIOSInfo();
	this->_FillBusInfo();
	this->_FillMemoryTypeInfo();
	
	this->_FillFanInfo();
	this->_FillClockInfo();
	this->_FillDefaultAndBoostClockInfo();
	this->_FillCoreTemp();
	this->_FillCoreVoltage();
}

void AMD_GPU::UpdateSensors() {
	this->_FillFanInfo();
	this->_FillClockInfo();
	this->_FillCoreTemp();
	this->_FillCoreVoltage();
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
	if (!this->_ADL_ADAPTER_MEMORYINFO_GET) return false;

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

bool AMD_GPU::_GetCurrentGeneration() {
	void* test = GetProcAddress(this->_hAModule, "ADL2_New_QueryPMLogData_Get");
	if (test) this->_currentGeneration = 8;
	else {
		test = GetProcAddress(this->_hAModule, "ADL2_OverdriveN_FanControl_Get");
		if (test) this->_currentGeneration = 7;
		else {
			test = GetProcAddress(this->_hAModule, "ADL_Overdrive6_FanSpeed_Get");
			if (test) this->_currentGeneration = 6;
			else {
				test = GetProcAddress(this->_hAModule, "ADL_Overdrive5_FanSpeed_Get");
				if (test) this->_currentGeneration = 5;
				else return false;
			}
		}
	}

	return true;
}

void AMD_GPU::_FillCoreVoltage() {
	this->Core.voltage = this->_overdrive->GetCoreVoltage();

	std::memmove(&this->_coreVoltageHistory[1], &this->_coreVoltageHistory[0], (GPUX_HISTORY_DEPTH - 1) * sizeof(double));
	this->_coreVoltageHistory[0] = this->Core.voltage;
}

void AMD_GPU::_FillCoreTemp() {
	this->Core.temperature = this->_overdrive->GetCoreTemp();

	std::memmove(&this->_coreTempHistory[1], &this->_coreTempHistory[0], (GPUX_HISTORY_DEPTH - 1) * sizeof(uint32_t));
	this->_coreTempHistory[0] = this->Core.temperature;
}

void AMD_GPU::_FillDefaultAndBoostClockInfo() {
	this->_overdrive->GetDefaultAndBoostClockInfo(&this->Core.defaultClock, &this->Core.boost, &this->Memory.defaultClock, &this->Memory.boost);
}

void AMD_GPU::_FillClockInfo() {
	this->_overdrive->GetClockInfo(&this->Memory.clock, &this->Core.clock);
}

void AMD_GPU::_FillMemoryTypeInfo() {
	ADLMemoryInfo adlMem;
	int result = this->_ADL_ADAPTER_MEMORYINFO_GET(this->_physAdapterIndex, &adlMem);
	if (ADL_OK == result) {
		std::string asciiString(adlMem.strMemoryType);
		this->Memory.memoryType = std::wstring(asciiString.begin(), asciiString.end());
	}
	else this->Memory.memoryType = L"UNKNOWN";
}

void AMD_GPU::_FillFanInfo() {
	auto currentFans = this->_overdrive->GetFanInfo();
	this->Fan.speedRpm = currentFans;

	if (_fanSpeedHistory.size() != currentFans.size())
		_fanSpeedHistory.resize(currentFans.size());

	for (size_t i = 0; i < currentFans.size(); i++) {
		auto& historyArray = _fanSpeedHistory[i];

		std::memmove(historyArray.data() + 1, historyArray.data(), (GPUX_HISTORY_DEPTH - 1) * sizeof(uint32_t));

		historyArray[0] = static_cast<uint32_t>(currentFans[i]);
	}
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