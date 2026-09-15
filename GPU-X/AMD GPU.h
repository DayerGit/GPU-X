#pragma once
#define AMD_VENDOR_ID 0x1002

#include "GPU.h"
#include "AMD/adl_common.h"

#include <vector>

class AMD_GPU : public GPU {
public:
    AMD_GPU(IDXGIAdapter* pDXGIAdapter, LUID AdapterLUID, int index, VkInstance vkInstance);
    
    void UpdateSensors();

    std::wstring GetBIOSVersion() const { return this->BIOS.version; }
    
    ~AMD_GPU();

private:
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

    HMODULE _hAModule;

    ADL_MAIN_CONTROL_CREATE _ADL_MAIN_CONTROL_CREATE;
    ADL_ADAPTER_NUMBEROFADAPTERS_GET _ADL_ADAPTER_NUMBEROFADAPTERS_GET;
    ADL_ADAPTER_ADAPTERINFO_GET _ADL_ADAPTER_ADAPTERINFO_GET;
    ADL_ADAPTER_VIDEOBIOSINFO_GET _ADL_ADAPTER_VIDEOBIOSINFO_GET;
    ADL_MAIN_CONTROL_DESTROY _ADL_MAIN_CONTROL_DESTROY;

    int _physAdapterIndex;

    bool _LoadLib();
    bool _GetDeviceHandle();

    void _FillBIOSInfo();
};
