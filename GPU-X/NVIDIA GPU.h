#pragma once
#define NVIDIA_VENDOR_ID 0x10DE

#include <vector>

#include "GPU.h"
#include "NVIDIA/nvapi.h"
#include "nvapi_common.h"

class NVIDIA_GPU : public GPU {
public:
    NVIDIA_GPU(IDXGIAdapter* pDXGIAdapter, LUID AdapterLUID, int index, VkInstance vkInstance);

    void UpdateSensors();

    std::wstring GetBIOSVersion() const { return this->BIOS.version; }
    std::wstring GetBusMaximum() const { return this->Bus.maximum; }
    std::wstring GetBusCurrent() const { return this->Bus.current; }
    std::wstring GetMemoryType() const { return this->Memory.memoryType; }

    std::vector<uint32_t> GetFanSpeed() const { return this->Fan.speedRpm; }

    uint32_t GetCoreTemperature() const { return this->Core.temperature; }
    uint32_t GetCoreClock() const { return this->Core.clock; }
    uint32_t GetCoreDefaultClock() const { return this->Core.defaultClock; }
    uint32_t GetCoreBoostClock() const { return this->Core.boost; }
    uint32_t GetMemoryClock() const { return this->Memory.clock; }
    uint32_t GetMemoryDefaultClock() const { return this->Memory.defaultClock; }
    uint32_t GetMemoryBoostClock() const { return this->Memory.boost; }

    double GetCoreVoltage() const { return this->Core.voltage; }

    double GetHasCUDA() const { return this->_hasCUDA; }
    double GetHasPhysX() const { return this->_hasPhysX; }

    ~NVIDIA_GPU();

private:
    bool _hasCUDA, _hasPhysX;

    struct {
        int32_t clock, defaultClock, boost, temperature;
        double voltage;
    } Core;

    struct {
        double defaultClock, boost, clock;
        std::wstring memoryType;
    } Memory;

    struct {
        std::vector<uint32_t> speedRpm;
    } Fan;

    struct {
        std::wstring maximum;
        std::wstring current;
    } Bus;

    struct {
        std::wstring version;
    } BIOS;

    HMODULE _hNModule;

    NvAPI_QueryInterface_t _NvAPI_QueryInterface;

    NvAPI_Initialize_t _NvAPI_Initialize;
    NvAPI_EnumPhysicalGPUs_t _NvAPI_EnumPhysicalGPUs;
    NvAPI_GPU_GetAllClockFrequencies_t _NvAPI_GPU_GetAllClockFrequencies;
    NvAPI_GPU_GetDynamicPstatesInfoEx_t _NvAPI_GPU_GetDynamicPstatesInfoEx;
    NvAPI_GPU_GetPstates20_t _NvAPI_GPU_GetPstates20;
    NvAPI_GPU_GetThermalSettings_t _NvAPI_GPU_GetThermalSettings;
    NvAPI_GPU_ClientVoltRailsGetStatus_t _NvAPI_GPU_ClientVoltRailsGetStatus;
    NvAPI_GPU_GetRamType_t _NvAPI_GPU_GetRamType;
    NvAPI_ClientFanCoolersGetStatus_t _NvAPI_ClientFanCoolersGetStatus;
    NvAPI_GPU_GetVbiosVersionString_t _NvAPI_GPU_GetVbiosVersionString;
    NvAPI_GPU_GetAdapterIdFromPhysicalGpu_t _NvAPI_GPU_GetAdapterIdFromPhysicalGpu;
    NvAPI_GPU_GetBusType_t _NvAPI_GPU_GetBusType;
    NvAPI_GPU_GetCurrentPCIEDownstreamWidth_t _NvAPI_GPU_GetCurrentPCIEDownstreamWidth;
    NvAPI_GPU_GetPCIEInfo_t _NvAPI_GPU_GetPCIEInfo;
    NvAPI_GPU_CudaEnumComputeCapableGpus_t _NvAPI_GPU_CudaEnumComputeCapableGpus;
    NvAPI_Unload_t _NvAPI_Unload;

    NvPhysicalGpuHandle _physGpuHandle = nullptr;

    bool _LoadLib();
    bool _GetDeviceHandle();

    std::wstring _GetBusName(NV_GPU_BUS_TYPE busType);

    void _FillBIOSInfo();
    void _FillBusInfo();
    void _FillFanInfo();

    void _FillMemoryInfo();

    int32_t _GetClock(NV_GPU_PUBLIC_CLOCK_ID clockID, NV_GPU_CLOCK_FREQUENCIES_CLOCK_TYPE clockType);
    void _FillCoreInfo();

    void _FillCUDAPhysXInfo();
};