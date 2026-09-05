#pragma once
#define AMD_VENDOR_ID 0x1002

#include "GPU.h"


class AMD_GPU : public GPU {
public:
    AMD_GPU(IDXGIAdapter* pDXGIAdapter, LUID AdapterLUID, int index, VkInstance vkInstance);
    
    void UpdateSensors();
    
    ~AMD_GPU();


private:
    struct {
        uint32_t clock;      
        uint32_t socClock;   
        int32_t  temperature;
        int32_t  junction;   
        float    power;      
        float    voltage;    
    } Core;

    struct {
        uint32_t     clock;      
        int32_t      temperature;
        std::wstring memoryType; 
        uint32_t     busWidth;   
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
