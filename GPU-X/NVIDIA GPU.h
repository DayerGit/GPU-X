#pragma once
#define NVIDIA_VENDOR_ID 0x10DE

#include "GPU.h"


class NVIDIA_GPU : public GPU {
public:
    NVIDIA_GPU(IDXGIAdapter* pDXGIAdapter, LUID AdapterLUID, int index, VkInstance vkInstance);

    void UpdateSensors();

    ~NVIDIA_GPU();

private:
    struct {
        uint32_t clock;
        int32_t  temperature;
        int32_t  hotspot;
        float    power;
        float    voltage;
    } Core;

    struct {
        uint32_t     clock;
        int32_t      temperature;
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