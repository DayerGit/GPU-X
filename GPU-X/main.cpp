#include <iostream>

#include "GPUFactory.h"


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
        std::cout << std::endl;
    }
    return 0;
}