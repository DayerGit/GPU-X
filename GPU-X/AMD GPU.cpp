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
	this->_FillFanInfo();
	this->_FillMemoryTypeInfo();
	this->_FillClockInfo();
	this->_FillDefaultAndBoostClockInfo();
	this->_FillCoreTemp();
	this->_FillCoreVoltage();
}

void* __stdcall ADL_Main_Memory_Alloc(int iSize) {
	return malloc(iSize);
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

	this->_ADL2_OVERDRIVEN_FANCONTROL_GET = (ADL2_OVERDRIVEN_FANCONTROL_GET)GetProcAddress(this->_hAModule, "ADL2_OverdriveN_FanControl_Get");
	if (this->_ADL2_OVERDRIVEN_FANCONTROL_GET) this->_currentGeneration = 'N';
	else {
		this->_ADL_OVERDRIVE6_FANSPEED_GET = (ADL_OVERDRIVE6_FANSPEED_GET)GetProcAddress(this->_hAModule, "ADL_Overdrive6_FanSpeed_Get");
		if (this->_ADL_OVERDRIVE6_FANSPEED_GET) this->_currentGeneration = 6;
		else {
			this->_ADL_OVERDRIVE5_FANSPEED_GET = (ADL_OVERDRIVE5_FANSPEED_GET)GetProcAddress(this->_hAModule, "ADL_Overdrive5_FanSpeed_Get");
			if (this->_ADL_OVERDRIVE5_FANSPEED_GET) this->_currentGeneration = 5;
			else return false;
		}
	}

	switch (this->_currentGeneration) {
	case 5: {
		this->_ADL_OVERDRIVE5_CURRENTACTIVITY_GET = 
			(ADL_OVERDRIVE5_CURRENTACTIVITY_GET)GetProcAddress(this->_hAModule, "ADL_Overdrive5_CurrentActivity_Get");

		if (!this->_ADL_OVERDRIVE5_CURRENTACTIVITY_GET) return false;

		this->_ADL_OVERDRIVE5_ODPARAMETERS_GET = 
			(ADL_OVERDRIVE5_ODPARAMETERS_GET)GetProcAddress(this->_hAModule, "ADL_Overdrive5_ODParameters_Get");

		if (!this->_ADL_OVERDRIVE5_ODPARAMETERS_GET) return false;

		this->_ADL_OVERDRIVE5_ODPERFORMANCELEVELS_GET = 
			(ADL_OVERDRIVE5_ODPERFORMANCELEVELS_GET)GetProcAddress(this->_hAModule, "ADL_Overdrive5_ODPerformanceLevels_Get");

		if (!this->_ADL_OVERDRIVE5_ODPERFORMANCELEVELS_GET) return false;

		this->_ADL_OVERDRIVE5_TEMPERATURE_GET =
			(ADL_OVERDRIVE5_TEMPERATURE_GET)GetProcAddress(this->_hAModule, "ADL_Overdrive5_Temperature_Get");

		if (!this->_ADL_OVERDRIVE5_TEMPERATURE_GET) return false;
		break;
	}
	case 6: {
		this->_ADL_OVERDRIVE6_CURRENTSTATUS_GET =
			(ADL_OVERDRIVE6_CURRENTSTATUS_GET)GetProcAddress(this->_hAModule, "ADL_Overdrive6_CurrentStatus_Get");

		if (!this->_ADL_OVERDRIVE6_CURRENTSTATUS_GET) return false;

		this->_ADL_OVERDRIVE6_CAPABILITIES_GET = 
			(ADL_OVERDRIVE6_CAPABILITIES_GET)GetProcAddress(this->_hAModule, "ADL_Overdrive6_Capabilities_Get");

		if (!this->_ADL_OVERDRIVE6_CAPABILITIES_GET) return false;

		this->_ADL_OVERDRIVE6_STATEINFO_GET =
			(ADL_OVERDRIVE6_STATEINFO_GET)GetProcAddress(this->_hAModule, "ADL_Overdrive6_StateInfo_Get");

		if (!this->_ADL_OVERDRIVE6_STATEINFO_GET) return false;

		this->_ADL_OVERDRIVE6_TEMPERATURE_GET =
			(ADL_OVERDRIVE6_TEMPERATURE_GET)GetProcAddress(this->_hAModule, "ADL_Overdrive6_Temperature_Get");

		if (!this->_ADL_OVERDRIVE6_TEMPERATURE_GET) return false;

		this->_ADL_OVERDRIVE6_VOLTAGECONTROL_GET =
			(ADL_OVERDRIVE6_VOLTAGECONTROL_GET)GetProcAddress(this->_hAModule, "ADL_Overdrive6_VoltageControl_Get");

		if (!this->_ADL_OVERDRIVE6_VOLTAGECONTROL_GET) return false;
		break;
	}
	case 'N': {
		break;
	}
	case 8: {
		break;
	}
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

void AMD_GPU::_FillCoreVoltage() {
	int result = ADL_OK;

	switch (this->_currentGeneration) {
	case 5: {
		ADLPMActivity adlPM = { 0 };
		adlPM.iSize = sizeof(adlPM);
		result = this->_ADL_OVERDRIVE5_CURRENTACTIVITY_GET(this->_physAdapterIndex, &adlPM);
		if (ADL_OK == result) {
			this->Core.voltage = adlPM.iVddc / 1000.0;
		}
		break;
	}
	case 6: {
		ADLOD6Capabilities od6Caps = { 0 };
		result = this->_ADL_OVERDRIVE6_CAPABILITIES_GET(this->_physAdapterIndex, &od6Caps);

		int currentValue = 0;
		int defaultValue = 0;

		this->_ADL_OVERDRIVE6_VOLTAGECONTROL_GET(this->_physAdapterIndex, &currentValue, &defaultValue);

		this->Core.voltage = currentValue / 1000.0;
		break;
	}
	case 'N': {
		break;
	}
	case 8: {
		break;
	}
	}
}

void AMD_GPU::_FillCoreTemp() {
	switch (this->_currentGeneration) {
	case 5: {
		ADLTemperature adlTemp = { 0 };
		adlTemp.iSize = sizeof(adlTemp);

		this->_ADL_OVERDRIVE5_TEMPERATURE_GET(this->_physAdapterIndex, 0, &adlTemp);

		this->Core.temperature = adlTemp.iTemperature / 1000;
		break;
	}
	case 6: {
		this->_ADL_OVERDRIVE6_TEMPERATURE_GET(this->_physAdapterIndex, &this->Core.temperature);
		this->Core.temperature /= 1000;
		break;
	}
	case 'N': {
		break;
	}
	case 8: {
		break;
	}
	}
}

void AMD_GPU::_FillDefaultAndBoostClockInfo() {
	int result = ADL_OK;
	switch (this->_currentGeneration) {
	case 5: {
		ADLODParameters odParams = { 0 };
		odParams.iSize = sizeof(ADLODParameters);

		result = this->_ADL_OVERDRIVE5_ODPARAMETERS_GET(this->_physAdapterIndex, &odParams);
		if (result == ADL_OK) {
			auto totalLevels = odParams.iNumberOfPerformanceLevels;
			int structureSize = sizeof(ADLODPerformanceLevels) + sizeof(ADLODPerformanceLevel) * (totalLevels - 1);
			ADLODPerformanceLevels* allocLevels = (ADLODPerformanceLevels*)calloc(1, structureSize);
			if (!allocLevels) break;
			allocLevels->iSize = structureSize;

			this->_ADL_OVERDRIVE5_ODPERFORMANCELEVELS_GET(this->_physAdapterIndex, 1, allocLevels);
			
			this->Core.defaultClock = allocLevels->aLevels[0].iEngineClock / 100.0;
			this->Core.boost = allocLevels->aLevels[totalLevels - 1].iEngineClock / 100.0;

			this->Memory.defaultClock = allocLevels->aLevels[0].iMemoryClock / 100.0;
			this->Memory.boost = allocLevels->aLevels[totalLevels - 1].iMemoryClock / 100.0;

			free(allocLevels);
		}
		break;
	}
	case 6: {
		ADLOD6Capabilities od6Caps = { 0 };

		result = this->_ADL_OVERDRIVE6_CAPABILITIES_GET(this->_physAdapterIndex, &od6Caps);
		if (result == ADL_OK) {
			auto totalLevels = od6Caps.iNumberOfPerformanceLevels;
			int structSize = sizeof(ADLOD6StateInfo) + sizeof(ADLOD6PerformanceLevel) * (totalLevels - 1);
			ADLOD6StateInfo* stateInfo = (ADLOD6StateInfo*)calloc(1, structSize);
			if (!stateInfo) break;
			stateInfo->iNumberOfPerformanceLevels = totalLevels;

			this->_ADL_OVERDRIVE6_STATEINFO_GET(this->_physAdapterIndex, 1, stateInfo);

			this->Core.defaultClock = stateInfo->aLevels[0].iEngineClock / 100.0;
			this->Core.boost = stateInfo->aLevels[totalLevels - 1].iEngineClock / 100.0;

			this->Memory.defaultClock = stateInfo->aLevels[0].iMemoryClock / 100.0;
			this->Memory.boost = stateInfo->aLevels[totalLevels - 1].iMemoryClock / 100.0;

			free(stateInfo);
		}
		break;
	}
	case 'N': {
		break;
	}
	case 8: {
		break;
	}
	}
}

void AMD_GPU::_FillClockInfo() {
	int result = ADL_OK;
	switch (this->_currentGeneration) {
	case 5: {
		ADLPMActivity adlPM = { 0 };
		adlPM.iSize = sizeof(adlPM);
		result = this->_ADL_OVERDRIVE5_CURRENTACTIVITY_GET(this->_physAdapterIndex, &adlPM);
		if (ADL_OK == result) {
			this->Memory.clock = adlPM.iMemoryClock / 100.0;
			this->Core.clock = adlPM.iEngineClock / 100.0;
		}
		break;
	}
	case 6: {
		ADLOD6CurrentStatus adlodCS = { 0 };
		result = this->_ADL_OVERDRIVE6_CURRENTSTATUS_GET(this->_physAdapterIndex, &adlodCS);
		if (ADL_OK == result) {
			this->Memory.clock = adlodCS.iMemoryClock / 100.0;
			this->Core.clock = adlodCS.iEngineClock / 100.0;
		}
		break;
	}
	case 'N': {
		break;
	}
	case 8: {
		break;
	}
	}
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
	int result = ADL_OK;
	this->Fan.speedRpm.clear();

	switch (this->_currentGeneration) {
	case 5: {
		ADLFanSpeedValue adlFSV = { 0 };
		adlFSV.iSize = sizeof(adlFSV);
		adlFSV.iSpeedType = ADL_DL_FANCTRL_SPEED_TYPE_RPM;
		result = this->_ADL_OVERDRIVE5_FANSPEED_GET(this->_physAdapterIndex, 0, &adlFSV);
		if(ADL_OK == result)
			this->Fan.speedRpm.push_back(adlFSV.iFanSpeed);
		break;
	}
	case 6: {
		ADLOD6FanSpeedInfo adlod6FSI = { 0 };
		adlod6FSI.iSpeedType = ADL_OD6_FANSPEED_TYPE_RPM;
		result = this->_ADL_OVERDRIVE6_FANSPEED_GET(this->_physAdapterIndex, &adlod6FSI);
		if (ADL_OK == result)
			this->Fan.speedRpm.push_back(adlod6FSI.iFanSpeedRPM);
		break;
	}
	case 'N': {
		break;
	}
	case 8: {
		break;
	}
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