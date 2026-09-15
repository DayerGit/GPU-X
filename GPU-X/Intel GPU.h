#pragma once
#define INTEL_VENDOR_ID 0x8086

#include "Intel/igcl_api.h"

#include "GPU.h"


class Intel_GPU : public GPU {
public:
    Intel_GPU(IDXGIAdapter* pDXGIAdapter, LUID AdapterLUID, int index, VkInstance vkInstance);
    
    void UpdateSensors() override;

    std::wstring GetBIOSVersion() const override { return this->BIOS.version; }
    std::wstring GetMemoryType() const override { return this->Memory.memoryType; }
    std::wstring GetBusMaximum() const override { return this->Bus.maximum; }
    std::wstring GetBusCurrent() const override { return this->Bus.current; }

    std::vector<uint32_t> GetFanSpeed() const { return std::vector<uint32_t>(this->Fan.speedRpm); }

    uint32_t GetCoreClock() const override { return uint32_t(this->Core.clock); }
    uint32_t GetDefaultCoreClock() const override { return uint32_t(this->Core.defaultClock); }
    uint32_t GetBoostCoreClock() const override { return uint32_t(this->Core.boost); }
    uint32_t GetDefaultMemoryClock() const override { return uint32_t(this->Memory.defaultClock); }
    uint32_t GetBoostMemoryClock() const override { return uint32_t(this->Memory.boost); }
    uint32_t GetMemoryClock() const override { return uint32_t(this->Memory.clock); }
    uint32_t GetCoreTemperature() const override { return uint32_t(this->Core.temperature); }

    double GetCoreVoltage() const override { return double(this->Core.voltage); }

    bool GetHasCUDA() const override { return false; }
    bool GetHasPhysX() const override { return false; }

    ~Intel_GPU();

private:
    struct {
        double clock, defaultClock, boost;        
        int8_t temperature;  
        int8_t voltage;
    } Core;

    struct {
        double defaultClock, boost;
        int8_t clock;
        std::wstring memoryType;  
    } Memory;

    struct {
        uint32_t speedRpm;     
    } Fan;

    struct {
        std::wstring maximum; 
        std::wstring current; 
    } Bus;

    struct {
        std::wstring version;
    } BIOS;

    ctl_api_handle_t _api_handle = nullptr;
    ctl_device_adapter_handle_t _device_adapter_handle;

    bool _GetDeviceHandle();

    std::wstring _GetBIOSVersion(ctl_firmware_version_t firmware_verison);
    std::wstring _MemTypeToString(ctl_mem_type_t mem_type);
    
    void _FillFromDeviceProperties();
    void _FillFromMemoryProperties();
    void _FillFreq();
    void _FillBus();
};
