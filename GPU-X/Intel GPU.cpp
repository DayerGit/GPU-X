#include "Intel GPU.h"

#include <vector>
#include <iostream>

Intel_GPU::Intel_GPU(IDXGIAdapter* pDXGIAdapter, LUID AdapterLUID, int index, VkInstance vkInstance) 
	: GPU(pDXGIAdapter, AdapterLUID, index, vkInstance) 
{
	this->_whoIsMyDaddy = TypeOfGPU::INTEL_GPU;

    ctl_init_args_t init_args = { 0 };
    init_args.Size = sizeof(ctl_init_args_t);
    init_args.Version = 0;
    init_args.AppVersion = CTL_MAKE_VERSION(CTL_IMPL_MAJOR_VERSION, CTL_IMPL_MINOR_VERSION);
    init_args.flags = CTL_INIT_FLAG_USE_LEVEL_ZERO;

    ctl_result_t result = ctlInit(&init_args, &this->_api_handle);

    if (result != CTL_RESULT_SUCCESS) return;

    if (!this->_GetDeviceHandle()) return;

    this->_FillFromDeviceProperties();
    this->_FillFromMemoryProperties();
    this->_FillFreq();
    this->_FillBus();

}

void Intel_GPU::UpdateSensors() {
    ctl_power_telemetry_t telemetry_power_data = { 0 };
    telemetry_power_data.Size = sizeof(ctl_power_telemetry_t);

    ctl_result_t result = ctlPowerTelemetryGet(this->_device_adapter_handle, &telemetry_power_data);
    if (result == CTL_RESULT_SUCCESS) {
        this->Core.clock = telemetry_power_data.gpuCurrentClockFrequency.value.datadouble;
        this->Core.temperature = telemetry_power_data.gpuCurrentTemperature.value.data8;
        this->Core.voltage = telemetry_power_data.gpuVoltage.value.data8;

        this->Memory.clock = telemetry_power_data.vramCurrentClockFrequency.value.data8;

        this->Fan.speedRpm = telemetry_power_data.fanSpeed[0].value.data32;
    }

    ctl_pci_state_t pci_state = { 0 };
    pci_state.Size = sizeof(ctl_pci_state_t);

    ctl_result_t res_state = ctlPciGetState(this->_device_adapter_handle, &pci_state);

    if (res_state == CTL_RESULT_SUCCESS && pci_state.speed.width != -1) {
        std::wstringstream ws;
        ws << L"PCIe x" << pci_state.speed.width << L" " << pci_state.speed.gen << L".0";
        this->Bus.current = ws.str();
    }
    else this->Bus.current = L"UNKNOWN";
}

bool Intel_GPU::_GetDeviceHandle() {
    uint32_t device_count = 0;
    ctl_result_t result = ctlEnumerateDevices(this->_api_handle, &device_count, nullptr);

    if (result == CTL_RESULT_SUCCESS && device_count > 0) {
        std::vector<ctl_device_adapter_handle_t> devices(device_count);
        result = ctlEnumerateDevices(this->_api_handle, &device_count, devices.data());

        if (result == CTL_RESULT_SUCCESS) {
            for (uint32_t i = 0; i < device_count; i++) {
                ctl_device_adapter_properties_t device_properties = { 0 };
                device_properties.Size = sizeof(ctl_device_adapter_properties_t);
                device_properties.Version = 2;

                device_properties.pDeviceID = new LUID[1]();
                device_properties.device_id_size = sizeof(LUID);

                result = ctlGetDeviceProperties(devices[i], &device_properties);

                if (result == CTL_RESULT_SUCCESS) {
                    if (!memcmp(&this->_adapterLUID, device_properties.pDeviceID, sizeof(LUID))) {
                        this->_device_adapter_handle = devices[i];
                        break;
                    }
                }

                delete[] device_properties.pDeviceID;
            }
        }
    }
    
    return this->_device_adapter_handle != nullptr;
}

std::wstring Intel_GPU::_GetBIOSVersion(ctl_firmware_version_t firmware_verison) {
    std::wstringstream ss;
    ss << firmware_verison.major_version << "." << firmware_verison.minor_version << "." << firmware_verison.build_number;
    return ss.str();
}

void Intel_GPU::_FillFromDeviceProperties() {
    ctl_device_adapter_properties_t device_properties = { 0 };
    device_properties.Size = sizeof(ctl_device_adapter_properties_t);
    device_properties.Version = 2;

    ctl_result_t result = ctlGetDeviceProperties(this->_device_adapter_handle, &device_properties);
    if (result == CTL_RESULT_SUCCESS) {
        this->BIOS.version = this->_GetBIOSVersion(device_properties.firmware_version);
    }
}

std::wstring Intel_GPU::_MemTypeToString(ctl_mem_type_t mem_type) {
    switch (mem_type) {
    case CTL_MEM_TYPE_HBM: return L"HBM";
    case CTL_MEM_TYPE_DDR: return L"DDR";
    case CTL_MEM_TYPE_DDR3: return L"DDR3";
    case CTL_MEM_TYPE_DDR4: return L"DDR4";
    case CTL_MEM_TYPE_DDR5: return L"DDR5";
    case CTL_MEM_TYPE_LPDDR: return L"LPDDR";
    case CTL_MEM_TYPE_LPDDR3: return L"LPDDR3";
    case CTL_MEM_TYPE_LPDDR4: return L"LPDDR4";
    case CTL_MEM_TYPE_LPDDR5: return L"LPDDR5";
    case CTL_MEM_TYPE_GDDR4: return L"GDDR4";
    case CTL_MEM_TYPE_GDDR5: return L"GDDR5";
    case CTL_MEM_TYPE_GDDR5X: return L"GDDR5X";
    case CTL_MEM_TYPE_GDDR6: return L"GDDR6";
    case CTL_MEM_TYPE_GDDR6X: return L"GDDR6X";
    case CTL_MEM_TYPE_GDDR7: return L"GDDR7";
    case CTL_MEM_TYPE_UNKNOWN: return L"UNKNOWN";
    }
    return L"UNKNOWN";
}

void Intel_GPU::_FillFromMemoryProperties() {
    uint32_t mem_modules_count = 0;

    ctl_result_t result = ctlEnumMemoryModules(this->_device_adapter_handle, &mem_modules_count, nullptr);

    if (result == CTL_RESULT_SUCCESS && mem_modules_count > 0) {
        std::vector<ctl_mem_handle_t> mem_modules(mem_modules_count);
        result = ctlEnumMemoryModules(this->_device_adapter_handle, &mem_modules_count, mem_modules.data());

        if (result == CTL_RESULT_SUCCESS) {
            for (uint32_t i = 0; i < mem_modules_count; i++) {
                ctl_mem_properties_t mem_properties = { 0 };
                mem_properties.Size = sizeof(ctl_mem_properties_t);

                result = ctlMemoryGetProperties(mem_modules[i], &mem_properties);

                if (result == CTL_RESULT_SUCCESS) {
                    this->Memory.memoryType = _MemTypeToString(mem_properties.type);

                    if (mem_properties.location == CTL_MEM_LOC_DEVICE) break;
                }
            }
        }
    }
}

void Intel_GPU::_FillFreq() {
    this->UpdateSensors();

    uint32_t frequency_handle_count = 0;
    ctlEnumFrequencyDomains(this->_device_adapter_handle, &frequency_handle_count, nullptr);

    if (frequency_handle_count > 0) {
        std::vector<ctl_freq_handle_t> hFrequency(frequency_handle_count);
        ctlEnumFrequencyDomains(this->_device_adapter_handle, &frequency_handle_count, hFrequency.data());

        for (uint32_t i = 0; i < frequency_handle_count; i++) {
            ctl_freq_properties_t freq_properties = { 0 };
            freq_properties.Size = sizeof(ctl_freq_properties_t);

            if (ctlFrequencyGetProperties(hFrequency[i], &freq_properties) == CTL_RESULT_SUCCESS) {
                if (freq_properties.type == CTL_FREQ_DOMAIN_GPU) {
                    this->Core.defaultClock = freq_properties.min;
                    this->Core.boost = freq_properties.max;
                }
                if (freq_properties.type == CTL_FREQ_DOMAIN_MEMORY) {
                    this->Memory.defaultClock = freq_properties.min;
                    this->Memory.boost = freq_properties.max;
                }
            }
        }
    }

}

void Intel_GPU::_FillBus() {
    ctl_pci_properties_t pci_props = { 0 };
    pci_props.Size = sizeof(ctl_pci_properties_t);
    pci_props.Version = 1;

    ctl_result_t res_props = ctlPciGetProperties(this->_device_adapter_handle, &pci_props);

    if (res_props == CTL_RESULT_SUCCESS) {
        std::wstringstream ws;
        ws << L"PCIe x" << pci_props.maxSpeed.width << L" " << pci_props.maxSpeed.gen << L".0";
        this->Bus.maximum = ws.str();
    }
    else this->Bus.maximum = L"UNKNOWN";
}

Intel_GPU::~Intel_GPU() {
    if(this->_api_handle)
        ctlClose(this->_api_handle);
}