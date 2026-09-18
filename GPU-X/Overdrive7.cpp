#include "Overdrive7.h"

Overdrive7::Overdrive7(HMODULE _hAModule, int _physAdapterIndex) {
    this->_hAModule = _hAModule;
    this->_physAdapterIndex = _physAdapterIndex;
}

bool Overdrive7::LoadLib() {
    this->_ADL2_MAIN_CONTROL_CREATE =
        (ADL2_MAIN_CONTROL_CREATE)GetProcAddress(this->_hAModule, "ADL2_Main_Control_Create");
    if (!this->_ADL2_MAIN_CONTROL_CREATE) return false;

    this->_ADL2_MAIN_CONTROL_DESTROY =
        (ADL2_MAIN_CONTROL_DESTROY)GetProcAddress(this->_hAModule, "ADL2_Main_Control_Destroy");
    if (!this->_ADL2_MAIN_CONTROL_DESTROY) return false;

    if (this->_ADL2_MAIN_CONTROL_CREATE(ADL_Main_Memory_Alloc, 1, &this->_context) != ADL_OK) return false;

    this->_ADL2_OVERDRIVE_CAPS =
        (ADL2_OVERDRIVE_CAPS)GetProcAddress(this->_hAModule, "ADL2_Overdrive_Caps");
    if (!this->_ADL2_OVERDRIVE_CAPS) return false;

    this->_ADL2_OVERDRIVEN_CAPABILITIES_GET =
        (ADL2_OVERDRIVEN_CAPABILITIES_GET)GetProcAddress(this->_hAModule, "ADL2_OverdriveN_Capabilities_Get");
    if (!this->_ADL2_OVERDRIVEN_CAPABILITIES_GET) return false;

    this->_ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET =
        (ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET)GetProcAddress(this->_hAModule, "ADL2_OverdriveN_PerformanceStatus_Get");
    if (!this->_ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET) return false;

    this->_ADL2_OVERDRIVEN_FANCONTROL_GET =
        (ADL2_OVERDRIVEN_FANCONTROL_GET)GetProcAddress(this->_hAModule, "ADL2_OverdriveN_FanControl_Get");
    if (!this->_ADL2_OVERDRIVEN_FANCONTROL_GET) return false;

    this->_ADL2_OVERDRIVEN_TEMPERATURE_GET =
        (ADL2_OVERDRIVEN_TEMPERATURE_GET)GetProcAddress(this->_hAModule, "ADL2_OverdriveN_Temperature_Get");
    if (!this->_ADL2_OVERDRIVEN_TEMPERATURE_GET) return false;

    this->_ADL2_OVERDRIVEN_SYSTEMCLOCKS_GET =
        (ADL2_OVERDRIVEN_SYSTEMCLOCKS_GET)GetProcAddress(this->_hAModule, "ADL2_OverdriveN_SystemClocks_Get");
    if (!this->_ADL2_OVERDRIVEN_SYSTEMCLOCKS_GET) return false;

    this->_ADL2_OVERDRIVEN_MEMORYCLOCKS_GET =
        (ADL2_OVERDRIVEN_MEMORYCLOCKS_GET)GetProcAddress(this->_hAModule, "ADL2_OverdriveN_MemoryClocks_Get");
    if (!this->_ADL2_OVERDRIVEN_MEMORYCLOCKS_GET) return false;

    return true;
}

double Overdrive7::GetCoreVoltage() {
    ADLODNPerformanceStatus status = { 0 };
    this->_ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET(this->_context, this->_physAdapterIndex, &status);
    return status.iVDDC / 1000.0;
}

int32_t Overdrive7::GetCoreTemp() {
    int temperature = 0;
    this->_ADL2_OVERDRIVEN_TEMPERATURE_GET(this->_context, this->_physAdapterIndex, 1, &temperature);
    return temperature / 1000;
}

void Overdrive7::GetDefaultAndBoostClockInfo(int32_t* defaultCoreClock, int32_t* boostCoreClock, double* defaultMemoryClock, double* boostMemoryClock) {
    ADLODNCapabilities caps = { 0 };
    int result = this->_ADL2_OVERDRIVEN_CAPABILITIES_GET(this->_context, this->_physAdapterIndex, &caps);
    if (ADL_OK != result) return;

    int totalLevels = caps.iMaximumNumberOfPerformanceLevels;
    if (totalLevels <= 0) return;

    size_t structSize = sizeof(ADLODNPerformanceLevels) + sizeof(ADLODNPerformanceLevel) * (totalLevels - 1);

    ADLODNPerformanceLevels* coreLevels = (ADLODNPerformanceLevels*)calloc(1, structSize);
    ADLODNPerformanceLevels* memLevels = (ADLODNPerformanceLevels*)calloc(1, structSize);
    if (!coreLevels || !memLevels) {
        free(coreLevels);
        free(memLevels);
        return;
    }

    coreLevels->iSize = memLevels->iSize = (int)structSize;

    result = this->_ADL2_OVERDRIVEN_SYSTEMCLOCKS_GET(this->_context, this->_physAdapterIndex, coreLevels);
    int numOfPerfLevels = coreLevels->iNumberOfPerformanceLevels;
    if (ADL_OK == result && numOfPerfLevels > 0) {
        *defaultCoreClock = coreLevels->aLevels[0].iClock / 100;
        *boostCoreClock = coreLevels->aLevels[numOfPerfLevels - 1].iClock / 100;
    }

    result = this->_ADL2_OVERDRIVEN_MEMORYCLOCKS_GET(this->_context, this->_physAdapterIndex, memLevels);
    int numOfMemPerfLevels = memLevels->iNumberOfPerformanceLevels;
    if (ADL_OK == result && numOfMemPerfLevels > 0) {
        *defaultMemoryClock = memLevels->aLevels[0].iClock / 100.0;
        *boostMemoryClock = memLevels->aLevels[numOfMemPerfLevels - 1].iClock / 100.0;
    }

    free(coreLevels);
    free(memLevels);
}

void Overdrive7::GetClockInfo(double* memory, int32_t* core) {
    ADLODNPerformanceStatus status = { 0 };
    int result = this->_ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET(this->_context, this->_physAdapterIndex, &status);
    if (ADL_OK == result) {
        *memory = status.iMemoryClock / 100.0;
        *core = status.iCoreClock / 100;
    }
}

std::vector<uint32_t> Overdrive7::GetFanInfo() {
    ADLODNFanControl fanControl = { 0 };
    std::vector<uint32_t> speedRpm;

    int result = this->_ADL2_OVERDRIVEN_FANCONTROL_GET(this->_context, this->_physAdapterIndex, &fanControl);
    if (ADL_OK == result)
        speedRpm.push_back(fanControl.iCurrentFanSpeed);

    return speedRpm;
}

Overdrive7::~Overdrive7() {
    if (this->_context && this->_ADL2_MAIN_CONTROL_DESTROY)
        this->_ADL2_MAIN_CONTROL_DESTROY(this->_context);
}