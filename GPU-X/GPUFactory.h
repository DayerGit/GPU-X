#pragma once

#include <vector>
#include <memory>

#include "GPU.h"
#include "NVIDIA GPU.h"
#include "AMD GPU.h"
#include "Intel GPU.h"

class GPUFactory {
public:
    GPUFactory();

    std::vector<std::unique_ptr<GPU>> LetsCreateGPUs();

    ~GPUFactory();
    
private:
    IDXGIFactory6* _pDXGIFactory = nullptr;
};
