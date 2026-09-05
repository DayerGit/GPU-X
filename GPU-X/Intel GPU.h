#pragma once
#define INTEL_VENDOR_ID 0x8086

#include "GPU.h"


class Intel_GPU : public GPU {
public:
    Intel_GPU(IDXGIAdapter* pDXGIAdapter, LUID AdapterLUID, int index, VkInstance vkInstance);
    
    void UpdateSensors();

    ~Intel_GPU();

private:
    struct {
        uint32_t clock;        
        int32_t  temperature;  
        float    power;        
        float    voltage;      
    } Core;

    struct {
        uint32_t     clock;       
        std::wstring memoryType;  
    } Memory;

    struct {
        uint32_t speedRpm;     
        uint32_t speedPercent; 
    } Fan;

    struct {
        std::wstring maximum; 
        std::wstring current; 
    } Bus;

    struct {
        std::wstring version;
    } BIOS;
};
