#pragma once
#define AMD_VENDOR_ID 0x1002

#include <vector>

#include "AMD/adl_common.h"

#include "GPU.h"
#include "Overdrive.h"
#include "Overdrive5.h"
#include "Overdrive6.h"
#include "Overdrive7.h"
#include "Overdrive8.h"


class AMD_GPU : public GPU {
public:
    AMD_GPU(IDXGIAdapter* pDXGIAdapter, LUID AdapterLUID, int index, VkInstance vkInstance);
    
    void UpdateSensors() override;

    std::wstring GetBIOSVersion() const override { return this->BIOS.version; }
    std::wstring GetBusMaximum() const override { return this->Bus.maximum; }
    std::wstring GetBusCurrent() const override { return this->Bus.current; }
    std::wstring GetMemoryType() const override { return this->Memory.memoryType; }

    std::vector<uint32_t> GetFanSpeed() const override { return this->Fan.speedRpm; }

    uint32_t GetCoreTemperature() const override { return this->Core.temperature; }
    uint32_t GetCoreClock() const override { return this->Core.clock; }
    uint32_t GetDefaultCoreClock() const override { return this->Core.defaultClock; }
    uint32_t GetBoostCoreClock() const override { return this->Core.boost; }
    uint32_t GetMemoryClock() const override { return this->Memory.clock; }
    uint32_t GetDefaultMemoryClock() const override { return this->Memory.defaultClock; }
    uint32_t GetBoostMemoryClock() const override { return this->Memory.boost; }

    double GetCoreVoltage() const override { return this->Core.voltage; }

    bool GetHasCUDA() const override { return false; }
    bool GetHasPhysX() const override { return false; }
    
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
    ADL_ADAPTER_MEMORYINFO_GET _ADL_ADAPTER_MEMORYINFO_GET;
    ADL_MAIN_CONTROL_DESTROY _ADL_MAIN_CONTROL_DESTROY;

    int _currentGeneration = 0;


    ADL2_OVERDRIVEN_FANCONTROL_GET _ADL2_OVERDRIVEN_FANCONTROL_GET;

    std::unique_ptr<Overdrive> _overdrive;

    int _physAdapterIndex;

    bool _LoadLib();
    bool _GetDeviceHandle();
    bool _GetCurrentGeneration();

    void _FillBIOSInfo();
    void _FillBusInfo();
    void _FillFanInfo();
    void _FillMemoryTypeInfo();
    void _FillClockInfo();
    void _FillDefaultAndBoostClockInfo();
    void _FillCoreTemp();
    void _FillCoreVoltage();

    std::wstring _FormatPcieString(ULONG speed, ULONG width);
};
