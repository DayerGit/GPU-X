#include <iostream>

#include "GPUFactory.h"

extern "C" {
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
}

int main() {
    GPUFactory factory;
    auto res = factory.LetsCreateGPUs();

    for (const auto& i : res) {
        std::wcout << std::setw(5) << std::setfill(L'=') << " " << i->GetDeviceName() << " " << std::setw(5) << std::setfill(L'=') << " " << std::endl;
        std::wcout << "  * DeviceID: " << i->GetDeviceID() << std::endl;
        std::wcout << "  * Revision: " << i->GetRevision() << std::endl;
        std::wcout << "  * DriverVersion: " << i->GetDriverVersion() << std::endl;
        std::wcout << "  * DriverDate: " << i->GetDriverDate() << std::endl;
        std::wcout << "  * MemSize: " << i->GetMemSize() << " MB" << std::endl;
        std::wcout << "  * D3D Version: " << i->GetDXMaxVersion() << std::endl;
        std::wcout << "  * RayTracing: " << i->GetHasRayTracing() << std::endl;
        std::wcout << "  * DirectCompute: " << i->GetHasDirectCompute() << std::endl;
        std::wcout << "  * DirectML: " << i->GetHasDirectML() << std::endl;
        std::wcout << "  * VULKAN: " << i->GetHasVulkan() << std::endl;
        std::wcout << "  * OpenCL: " << i->GetHasOpenCL() << std::endl;
        std::wcout << "  * OpenGL 4.6: " << i->GetHasOGL4_6() << std::endl;
        std::wcout << "  * Resizable BAR: " << i->GetHasResizableBAR() << std::endl;
        
        switch (i->GetProducer()) {
        case TypeOfGPU::INTEL_GPU: {
            Intel_GPU* intelGPU = static_cast<Intel_GPU*>(i.get());
            std::wcout << "  * BIOS Version: " << intelGPU->GetBIOSVersion() << std::endl;
            std::wcout << "  * Memory Type: " << intelGPU->GetMemoryType() << std::endl;
            std::wcout << "  * GPU Clock: " << intelGPU->GetCoreClock() << std::endl;
            std::wcout << "  * GPU Default Clock: " << intelGPU->GetDefaultCoreClock() << std::endl;
            std::wcout << "  * GPU Boost Clock: " << intelGPU->GetBoostCoreClock() << std::endl;
            std::wcout << "  * Memory Clock: " << intelGPU->GetMemoryClock() << std::endl;
            std::wcout << "  * Memory Default Clock: " << intelGPU->GetDefaultMemoryClock() << std::endl;
            std::wcout << "  * Memory Boost Clock: " << intelGPU->GetBoostMemoryClock() << std::endl;
            std::wcout << "  * Fan Speed: " << intelGPU->GetFanSpeed() << std::endl;
            std::wcout << "  * Bus Maximum: " << intelGPU->GetBusMaximum() << std::endl;
            std::wcout << "  * Bus Current: " << intelGPU->GetBusCurrent() << std::endl;
            break;
        }
        case TypeOfGPU::NVIDIA_GPU: {
            NVIDIA_GPU* NvidiaGPU = static_cast<NVIDIA_GPU*>(i.get());
            std::wcout << "  * BIOS Version: " << NvidiaGPU->GetBIOSVersion() << std::endl;
            std::wcout << "  * Bus Maximum: " << NvidiaGPU->GetBusMaximum() << std::endl;
            std::wcout << "  * Bus Current: " << NvidiaGPU->GetBusCurrent() << std::endl;
            std::wcout << "  * Memory Type: " << NvidiaGPU->GetMemoryType() << std::endl;
            std::wcout << "  * Core Temp: " << NvidiaGPU->GetCoreTemperature() << std::endl;
            std::wcout << "  * Core Clock: " << NvidiaGPU->GetCoreClock() << std::endl;
            std::wcout << "  * Core Default Clock: " << NvidiaGPU->GetCoreDefaultClock() << std::endl;
            std::wcout << "  * Core Boost Clock: " << NvidiaGPU->GetCoreBoostClock() << std::endl;
            std::wcout << "  * Core Voltage: " << NvidiaGPU->GetCoreVoltage() << " V" << std::endl;
            std::wcout << "  * Memory Clock: " << NvidiaGPU->GetMemoryClock() << std::endl;
            std::wcout << "  * Memory Default Clock: " << NvidiaGPU->GetMemoryDefaultClock() << std::endl;
            std::wcout << "  * Memory Boost Clock: " << NvidiaGPU->GetMemoryBoostClock() << std::endl;
            std::wcout << "  * CUDA: " << NvidiaGPU->GetHasCUDA() << std::endl;
            std::wcout << "  * PhysX: " << NvidiaGPU->GetHasPhysX() << std::endl;

            std::wcout << "  * Fan Speed: " << std::endl;
            const auto& fanSpeed = NvidiaGPU->GetFanSpeed();
            for (int i = 0; i < fanSpeed.size(); i++) {
                std::wcout << "    - Fan #" << i+1 << ": " << fanSpeed[i] << std::endl;

            }
            break;
        }
        case TypeOfGPU::AMD_GPU: {
            AMD_GPU* AmdGPU = static_cast<AMD_GPU*>(i.get());
            std::wcout << "  * BIOS Version: " << AmdGPU->GetBIOSVersion() << std::endl;
            break;
        }
        }

        std::cout << std::endl;
    }

    system("pause");
    return 0;
}