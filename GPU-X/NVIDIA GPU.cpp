#include "NVIDIA GPU.h"

#include <iostream>
#include <algorithm>

NVIDIA_GPU::NVIDIA_GPU(IDXGIAdapter* pDXGIAdapter, LUID AdapterLUID, int index, VkInstance vkInstance) 
						: GPU(pDXGIAdapter, AdapterLUID, index, vkInstance)
{
	this->_whoIsMyDaddy = TypeOfGPU::NVIDIA_GPU;

	if (!this->_LoadLib()) return;

	if (!this->_GetDeviceHandle()) return;

	this->_FillBIOSInfo();
	this->_FillBusInfo();
	this->_FillFanInfo();
	this->_FillMemoryInfo();
	this->_FillCoreInfo();
	this->_FillCUDAPhysXInfo();
}

void NVIDIA_GPU::UpdateSensors() {
	this->_FillFanInfo();
	this->_FillMemoryInfo();
	this->_FillCoreInfo();
}

bool NVIDIA_GPU::_LoadLib() {

#ifdef _WIN64
	this->_hNModule = LoadLibraryA("nvapi64.dll");
#else
	this->_hNModule = LoadLibraryA("nvapi.dll");
#endif

	if (!this->_hNModule) return false;

	this->_NvAPI_QueryInterface = (NvAPI_QueryInterface_t)GetProcAddress(this->_hNModule, "nvapi_QueryInterface");
	if (!this->_NvAPI_QueryInterface) return false;

	this->_NvAPI_Initialize = (NvAPI_Initialize_t)this->_NvAPI_QueryInterface(NvAPI_Initialize_ID);
	if (!this->_NvAPI_Initialize) return false;

	this->_NvAPI_EnumPhysicalGPUs = (NvAPI_EnumPhysicalGPUs_t)this->_NvAPI_QueryInterface(NvAPI_EnumPhysicalGPUs_ID);
	if (!this->_NvAPI_EnumPhysicalGPUs) return false;

	this->_NvAPI_GPU_GetAllClockFrequencies = (NvAPI_GPU_GetAllClockFrequencies_t)this->_NvAPI_QueryInterface(NvAPI_GPU_GetAllClockFrequencies_ID);
	if (!this->_NvAPI_GPU_GetAllClockFrequencies) return false;

	this->_NvAPI_GPU_GetDynamicPstatesInfoEx = (NvAPI_GPU_GetDynamicPstatesInfoEx_t)this->_NvAPI_QueryInterface(NvAPI_GPU_GetDynamicPstatesInfoEx_ID);
	if (!this->_NvAPI_GPU_GetDynamicPstatesInfoEx) return false;

	this->_NvAPI_GPU_GetPstates20 = (NvAPI_GPU_GetPstates20_t)this->_NvAPI_QueryInterface(NvAPI_GPU_GetPstates20_ID);
	if (!this->_NvAPI_GPU_GetPstates20) return false;

	this->_NvAPI_GPU_GetThermalSettings = (NvAPI_GPU_GetThermalSettings_t)this->_NvAPI_QueryInterface(NvAPI_GPU_GetThermalSettings_ID);
	if (!this->_NvAPI_GPU_GetThermalSettings) return false;

	this->_NvAPI_GPU_ClientVoltRailsGetStatus = (NvAPI_GPU_ClientVoltRailsGetStatus_t)this->_NvAPI_QueryInterface(NvAPI_GPU_ClientVoltRailsGetStatus_ID);
	if (!this->_NvAPI_GPU_ClientVoltRailsGetStatus) return false;

	this->_NvAPI_ClientFanCoolersGetStatus = (NvAPI_ClientFanCoolersGetStatus_t)this->_NvAPI_QueryInterface(NvAPI_ClientFanCoolersGetStatus_ID);
	if (!this->_NvAPI_ClientFanCoolersGetStatus) return false;

	this->_NvAPI_GPU_GetVbiosVersionString = (NvAPI_GPU_GetVbiosVersionString_t)this->_NvAPI_QueryInterface(NvAPI_GPU_GetVbiosVersionString_ID);
	if (!this->_NvAPI_GPU_GetVbiosVersionString) return false;

	this->_NvAPI_GPU_GetAdapterIdFromPhysicalGpu = (NvAPI_GPU_GetAdapterIdFromPhysicalGpu_t)this->_NvAPI_QueryInterface(NvAPI_GPU_GetAdapterIdFromPhysicalGpu_ID);
	if (!this->_NvAPI_GPU_GetAdapterIdFromPhysicalGpu) return false;

	this->_NvAPI_GPU_GetBusType = (NvAPI_GPU_GetBusType_t)this->_NvAPI_QueryInterface(NvAPI_GPU_GetBusType_ID);
	if (!this->_NvAPI_GPU_GetBusType) return false;

	this->_NvAPI_GPU_GetRamType = (NvAPI_GPU_GetRamType_t)this->_NvAPI_QueryInterface(NvAPI_GPU_GetRamType_ID);
	if (!this->_NvAPI_GPU_GetRamType) return false;

	this->_NvAPI_GPU_GetCurrentPCIEDownstreamWidth = (NvAPI_GPU_GetCurrentPCIEDownstreamWidth_t)this->_NvAPI_QueryInterface(NvAPI_GPU_GetCurrentPCIEDownstreamWidth_ID);
	if (!this->_NvAPI_GPU_GetCurrentPCIEDownstreamWidth) return false;

	this->_NvAPI_GPU_GetPCIEInfo = (NvAPI_GPU_GetPCIEInfo_t)this->_NvAPI_QueryInterface(NvAPI_GPU_GetPCIEInfo_ID);
	if (!this->_NvAPI_GPU_GetPCIEInfo) return false;

	this->_NvAPI_GPU_CudaEnumComputeCapableGpus = (NvAPI_GPU_CudaEnumComputeCapableGpus_t)this->_NvAPI_QueryInterface(NvAPI_GPU_CudaEnumComputeCapableGpus_ID);
	if (!this->_NvAPI_GPU_CudaEnumComputeCapableGpus) return false;

	this->_NvAPI_Unload = (NvAPI_Unload_t)this->_NvAPI_QueryInterface(NvAPI_Unload_ID);
	if (!this->_NvAPI_Unload) return false;

	NvAPI_Status result = _NvAPI_Initialize();
	if (result != NVAPI_OK) return false;

	return true;
}

bool NVIDIA_GPU::_GetDeviceHandle() {
	NvPhysicalGpuHandle gpuHandles[NVAPI_MAX_PHYSICAL_GPUS] = { 0 };
	NvU32 gpuCount = 0;

	NvAPI_Status status = this->_NvAPI_EnumPhysicalGPUs(gpuHandles, &gpuCount);
	if (status != NVAPI_OK) return false;

	for (NvU32 i = 0; i < gpuCount; i++) {
		LUID osAdapterId = { 0 };

		status = this->_NvAPI_GPU_GetAdapterIdFromPhysicalGpu(gpuHandles[i], static_cast<void*>(&osAdapterId));

		if (status == NVAPI_OK) {
			if (!memcmp(&osAdapterId, &this->_adapterLUID, sizeof(LUID))) {
				this->_physGpuHandle = gpuHandles[i];
				return true;
			}
		}
	}

	return false;
}

void NVIDIA_GPU::_FillBIOSInfo() {
	NvAPI_ShortString vbiosVersion = { 0 };
	NvAPI_Status result = this->_NvAPI_GPU_GetVbiosVersionString(this->_physGpuHandle, vbiosVersion);

	if (result == NVAPI_OK) {
		std::string asciiString(vbiosVersion);
		this->BIOS.version = std::wstring(asciiString.begin(), asciiString.end());
	}
}

std::wstring NVIDIA_GPU::_GetBusName(NV_GPU_BUS_TYPE busType) {
	switch (busType) {
	case NVAPI_GPU_BUS_TYPE_UNDEFINED: return L"N/A";
	case NVAPI_GPU_BUS_TYPE_PCI: return L"PCI";
	case NVAPI_GPU_BUS_TYPE_AGP: return L"AGP";
	case NVAPI_GPU_BUS_TYPE_PCI_EXPRESS: return L"PCIe";
	case NVAPI_GPU_BUS_TYPE_FPCI: return L"FPCI";
	case NVAPI_GPU_BUS_TYPE_AXI: return L"AXI";
	}
	return L"N/A";
}

void NVIDIA_GPU::_FillBusInfo() {
	NV_GPU_BUS_TYPE busType;
	this->_NvAPI_GPU_GetBusType(this->_physGpuHandle, &busType);

	NvU32 currentLanes = 0;
	this->_NvAPI_GPU_GetCurrentPCIEDownstreamWidth(this->_physGpuHandle, &currentLanes);

	std::wstring busName = this->_GetBusName(busType);
	std::wstring curGen = L"";
	std::wstring maxGen = L"";
	NvU32 maxLanes = 16;

	if (busType == NVAPI_GPU_BUS_TYPE_PCI_EXPRESS) {
		NV_PCIE_INFO pcieInfo = {};
		pcieInfo.version = NV_PCIE_INFO_VER;

		if (this->_NvAPI_GPU_GetPCIEInfo(this->_physGpuHandle, &pcieInfo) == NVAPI_OK) {
			NvU32 cg = pcieInfo.info[0].unknown1;
			if (cg >= 1 && cg <= 6) curGen = std::to_wstring(cg) + L".0";

			NvU32 mg = pcieInfo.info[1].unknown1;
			if (mg >= 1 && mg <= 6) maxGen = std::to_wstring(mg) + L".0";

			if (pcieInfo.info[1].unknown2 >= 1 && pcieInfo.info[1].unknown2 <= 16) {
				maxLanes = pcieInfo.info[1].unknown2;
			}
		}
	}

	switch (busType) {
	case NVAPI_GPU_BUS_TYPE_PCI_EXPRESS: {
		if (curGen.empty()) curGen = L"1.0";
		if (maxGen.empty()) maxGen = L"1.0";

		this->Bus.maximum = busName + L" " + maxGen + L" x" + std::to_wstring(maxLanes);
		this->Bus.current = busName + L" " + curGen + L" x" + std::to_wstring(currentLanes);
		break;
	}
	case NVAPI_GPU_BUS_TYPE_AGP: {
		this->Bus.maximum = busName + L" x" + std::to_wstring(currentLanes);
		this->Bus.current = busName + L" x" + std::to_wstring(currentLanes);
		break;
	}
	default: {
		this->Bus.maximum = busName;
		this->Bus.current = busName;
		break;
	}
	}
}

void NVIDIA_GPU::_FillFanInfo() {
	this->Fan.speedRpm.clear();

	NV_GPU_CLIENT_FAN_COOLERS_STATUS status = {};
	status.version = NV_GPU_CLIENT_FAN_COOLERS_STATUS_VER;

	NvAPI_Status result = this->_NvAPI_ClientFanCoolersGetStatus(this->_physGpuHandle, &status);
	if (result != NVAPI_OK) return;

	if (_fanSpeedHistory.size() != status.count)
		_fanSpeedHistory.resize(status.count);

	for (NvU32 i = 0; i < status.count; i++) {
		uint32_t currentRpm = status.items[i].currentRpm;
		this->Fan.speedRpm.push_back(currentRpm);

		auto& historyArray = _fanSpeedHistory[i];

		std::memmove(historyArray.data() + 1, historyArray.data(), (GPUX_HISTORY_DEPTH - 1) * sizeof(uint32_t));

		historyArray[0] = currentRpm;
	}
}


std::wstring GetMemoryTypeStr(unsigned int ramType) {
	switch (ramType) {
	case NV_RAM_UNKNOWN: return L"Unknown";
	case NV_RAM_SDRAM:   return L"SDRAM";
	case NV_RAM_DDR1:    return L"DDR1";
	case NV_RAM_DDR2:    return L"DDR2";
	case NV_RAM_GDDR2:   return L"GDDR2";
	case NV_RAM_GDDR3:   return L"GDDR3";
	case NV_RAM_GDDR4:   return L"GDDR4";
	case NV_RAM_DDR3:    return L"DDR3";
	case NV_RAM_GDDR5:   return L"GDDR5";
	case NV_RAM_LPDDR2:  return L"LPDDR2";
	case NV_RAM_GDDR5X:  return L"GDDR5X";
	case NV_RAM_HBM1:    return L"HBM";
	case NV_RAM_HBM2:    return L"HBM2";
	case NV_RAM_HBM2E:   return L"HBM2e";
	case NV_RAM_GDDR6:   return L"GDDR6";
	case NV_RAM_GDDR6X:  return L"GDDR6X";
	case NV_RAM_GDDR7:   return L"GDDR7";
	case NV_RAM_HBM3:    return L"HBM3";
	case NV_RAM_HBM3E:   return L"HBM3e";
	default:             return L"Generic DDR / Type " + std::to_wstring(ramType);
	}
}

int32_t NVIDIA_GPU::_GetClock(NV_GPU_PUBLIC_CLOCK_ID clockID, NV_GPU_CLOCK_FREQUENCIES_CLOCK_TYPE clockType) {
	NV_GPU_CLOCK_FREQUENCIES clocks = { 0 };
	clocks.version = NV_GPU_CLOCK_FREQUENCIES_VER;
	clocks.ClockType = clockType;

	NvAPI_Status result = this->_NvAPI_GPU_GetAllClockFrequencies(this->_physGpuHandle, &clocks);

	if (result == NVAPI_OK) 
		if (clocks.domain[clockID].bIsPresent)
			return clocks.domain[clockID].frequency / 1000;
	
	return 0.0;
}

void NVIDIA_GPU::_FillCoreInfo() {
	this->Core.defaultClock = this->_GetClock(NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS, NV_GPU_CLOCK_FREQUENCIES_BASE_CLOCK);
	this->Core.boost = this->_GetClock(NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS, NV_GPU_CLOCK_FREQUENCIES_BOOST_CLOCK);
	this->Core.clock = this->_GetClock(NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS, NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ);

	NV_GPU_THERMAL_SETTINGS thermalSettings = { 0 };
	thermalSettings.version = NV_GPU_THERMAL_SETTINGS_VER;
	NvAPI_Status result = this->_NvAPI_GPU_GetThermalSettings(this->_physGpuHandle, 0, &thermalSettings);

	if (result == NVAPI_OK) 
		this->Core.temperature = thermalSettings.sensor[0].currentTemp;

	NV_GPU_CLIENT_VOLT_RAILS_STATUS status = { 0 };
	status.version = NV_GPU_CLIENT_VOLT_RAILS_STATUS_VER;

	result = this->_NvAPI_GPU_ClientVoltRailsGetStatus(this->_physGpuHandle, &status);

	if (result == NVAPI_OK)
		this->Core.voltage = status.data[9] / 1000000.0f;

	std::memmove(&this->_coreTempHistory[1], &this->_coreTempHistory[0], (GPUX_HISTORY_DEPTH - 1) * sizeof(uint32_t));
	this->_coreTempHistory[0] = this->Core.temperature;

	std::memmove(&this->_coreVoltageHistory[1], &this->_coreVoltageHistory[0], (GPUX_HISTORY_DEPTH - 1) * sizeof(double));
	this->_coreVoltageHistory[0] = this->Core.voltage;
}

void NVIDIA_GPU::_FillMemoryInfo() {
	NvU32 ramType = 0;
	this->_NvAPI_GPU_GetRamType(this->_physGpuHandle, &ramType);
	this->Memory.memoryType = GetMemoryTypeStr(ramType);

	this->Memory.defaultClock = this->_GetClock(NVAPI_GPU_PUBLIC_CLOCK_MEMORY, NV_GPU_CLOCK_FREQUENCIES_BASE_CLOCK);
	this->Memory.boost = this->_GetClock(NVAPI_GPU_PUBLIC_CLOCK_MEMORY, NV_GPU_CLOCK_FREQUENCIES_BOOST_CLOCK);
	this->Memory.clock = this->_GetClock(NVAPI_GPU_PUBLIC_CLOCK_MEMORY, NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ);

	int dev = 1;

	switch (ramType) {
	case NV_RAM_GDDR5:
	case NV_RAM_GDDR5X: {
		dev = 2;
		break;
	}
	case NV_RAM_GDDR6: 
	case NV_RAM_GDDR6X: {
		dev = 4;
		break;
	}
	case NV_RAM_GDDR7: {
		dev = 8;
		break;
	}
	default: break;
	}

	this->Memory.defaultClock /= dev;
	this->Memory.boost /= dev;
	this->Memory.clock /= dev;
}

void NVIDIA_GPU::_FillCUDAPhysXInfo() {
	NV_COMPUTE_GPU_TOPOLOGY_V1 topo = { 0 };
	topo.version = NV_COMPUTE_GPU_TOPOLOGY_VER1;

	NvAPI_Status result = this->_NvAPI_GPU_CudaEnumComputeCapableGpus(&topo);
	if (result != NVAPI_OK || !topo.gpuCount) return;

	for (NvU32 i = 0; i < topo.gpuCount; i++) {
		if (topo.computeGpus[i].hPhysicalGpu != this->_physGpuHandle) continue;

		const NvU32 flags = topo.computeGpus[i].flags;

		this->_hasCUDA = (flags & NV_COMPUTE_GPU_TOPOLOGY_CUDA_DISABLED) == 0;
		this->_hasPhysX = (flags & NV_COMPUTE_GPU_TOPOLOGY_PHYSICS_CAPABLE) != 0;

		break;
	}
}

NVIDIA_GPU::~NVIDIA_GPU() {
	if(this->_NvAPI_Unload)
		this->_NvAPI_Unload();
}