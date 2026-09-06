#pragma once
#define INTEL_VENDOR_ID 0x8086

#include "Intel/igcl_api.h"

#include "GPU.h"


class Intel_GPU : public GPU {
public:
    Intel_GPU(IDXGIAdapter* pDXGIAdapter, LUID AdapterLUID, int index, VkInstance vkInstance);
    
    void UpdateSensors();

    std::wstring GetBIOSVersion() const { return this->BIOS.version; }
    std::wstring GetMemoryType() const { return this->Memory.memoryType; }
    std::wstring GetBusMaximum() const { return this->Bus.maximum; }
    std::wstring GetBusCurrent() const { return this->Bus.current; }

    double GetCoreClock() const { return this->Core.clock; }
    double GetDefaultCoreClock() const { return this->Core.defaultClock; }
    double GetBoostCoreClock() const { return this->Core.boost; }
    double GetCorePower() const { return this->Core.power; }

    double GetDefaultMemoryClock() const { return this->Memory.defaultClock; }
    double GetBoostMemoryClock() const { return this->Memory.boost; }

    int8_t GetMemoryClock() const { return this->Memory.clock; }
    int8_t GetCoreTemperature() const { return this->Core.temperature; }
    int8_t GetCoreVoltage() const { return this->Core.voltage; }

    int32_t GetFanSpeed() const { return this->Fan.speedRpm; }

    ~Intel_GPU();

private:
    struct {
        double clock, defaultClock, boost;        
        double power;        
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

    double _last_energy, _last_timestamp;

    bool _GetDeviceHandle();

    std::wstring _GetBIOSVersion(ctl_firmware_version_t firmware_verison);
    std::wstring _MemTypeToString(ctl_mem_type_t mem_type);
    
    void _FillFromDeviceProperties();
    void _FillFromMemoryProperties();
    void _FillFreq();
    void _FillBus();
};
