#include "Overdrive8.h"

Overdrive8::Overdrive8(HMODULE _hAModule, int _physAdapterIndex) {
    this->_hAModule = _hAModule;
    this->_physAdapterIndex = _physAdapterIndex;
}

bool Overdrive8::LoadLib() {
    this->_ADL2_MAIN_CONTROL_CREATE =
        (ADL2_MAIN_CONTROL_CREATE)GetProcAddress(this->_hAModule, "ADL2_Main_Control_Create");
    if (!this->_ADL2_MAIN_CONTROL_CREATE) return false;

    this->_ADL2_MAIN_CONTROL_DESTROY =
        (ADL2_MAIN_CONTROL_DESTROY)GetProcAddress(this->_hAModule, "ADL2_Main_Control_Destroy");
    if (!this->_ADL2_MAIN_CONTROL_DESTROY) return false;

    if (this->_ADL2_MAIN_CONTROL_CREATE(ADL_Main_Memory_Alloc, 1, &this->_context) != ADL_OK)
        return false;

    this->_ADL2_OVERDRIVE_CAPS =
        (ADL2_OVERDRIVE_CAPS)GetProcAddress(this->_hAModule, "ADL2_Overdrive_Caps");
    if (!this->_ADL2_OVERDRIVE_CAPS) return false;

    this->_ADL2_OVERDRIVE8_INIT_SETTING_GET =
        (ADL2_OVERDRIVE8_INIT_SETTING_GET)GetProcAddress(this->_hAModule, "ADL2_Overdrive8_Init_Setting_Get");
    if (!this->_ADL2_OVERDRIVE8_INIT_SETTING_GET) return false;

    this->_ADL2_OVERDRIVE8_CURRENT_SETTING_GET =
        (ADL2_OVERDRIVE8_CURRENT_SETTING_GET)GetProcAddress(this->_hAModule, "ADL2_Overdrive8_Current_Setting_Get");
    if (!this->_ADL2_OVERDRIVE8_CURRENT_SETTING_GET) return false;

    this->_ADL2_NEW_QUERYPMLOGDATA_GET =
        (ADL2_NEW_QUERYPMLOGDATA_GET)GetProcAddress(this->_hAModule, "ADL2_New_QueryPMLogData_Get");
    if (!this->_ADL2_NEW_QUERYPMLOGDATA_GET) return false;

    int result = this->_ADL2_OVERDRIVE8_INIT_SETTING_GET(this->_context, this->_physAdapterIndex, &this->_initSetting);
    this->_initSettingValid = (ADL_OK == result);

    return true;
}

double Overdrive8::GetCoreVoltage() {
    ADLPMLogDataOutput data = { 0 };
    int result = this->_ADL2_NEW_QUERYPMLOGDATA_GET(this->_context, this->_physAdapterIndex, &data);
    if (ADL_OK != result) return 0.0;

    const ADLSingleSensorData& gfxVoltage = data.sensors[PMLOG_GFX_VOLTAGE];
    if (!gfxVoltage.supported) return 0.0;

    return gfxVoltage.value / 1000.0;
}

int32_t Overdrive8::GetCoreTemp() {
    ADLPMLogDataOutput data = { 0 };
    int result = this->_ADL2_NEW_QUERYPMLOGDATA_GET(this->_context, this->_physAdapterIndex, &data);
    if (ADL_OK != result) return 0;

    const ADLSingleSensorData& edgeTemp = data.sensors[PMLOG_TEMPERATURE_EDGE];
    if (!edgeTemp.supported) return 0;

    return edgeTemp.value;
}

void Overdrive8::GetDefaultAndBoostClockInfo(int32_t* defaultCoreClock, int32_t* boostCoreClock, double* defaultMemoryClock, double* boostMemoryClock) {
    if (!this->_initSettingValid) return;

    for (int i = 0; i < this->_initSetting.count && i < OD8_COUNT; i++) {
        const ADLOD8SingleInitSetting& setting = this->_initSetting.od8SettingTable[i];

        switch (setting.featureID) {
        case OD8_GFXCLK_FMIN: {
            *defaultCoreClock = setting.defaultValue;
            break;
        }
        case OD8_GFXCLK_FMAX: {
            *boostCoreClock = setting.defaultValue;
            break;
        }
        case OD8_UCLK_FMIN: {
            *defaultMemoryClock = setting.defaultValue;
            break;
        }
        case OD8_UCLK_FMAX: {
            *boostMemoryClock = setting.defaultValue;
            break;
        }
        }
    }
}

void Overdrive8::GetClockInfo(double* memory, int32_t* core) {
    ADLPMLogDataOutput data = { 0 };
    int result = this->_ADL2_NEW_QUERYPMLOGDATA_GET(this->_context, this->_physAdapterIndex, &data);
    if (ADL_OK != result) return;

    const ADLSingleSensorData& gfxClk = data.sensors[PMLOG_CLK_GFXCLK];
    const ADLSingleSensorData& memClk = data.sensors[PMLOG_CLK_MEMCLK];

    if (gfxClk.supported) *core = gfxClk.value;
    if (memClk.supported) *memory = memClk.value;
}

std::vector<uint32_t> Overdrive8::GetFanInfo() {
    std::vector<uint32_t> speedRpm;

    ADLPMLogDataOutput data = { 0 };
    int result = this->_ADL2_NEW_QUERYPMLOGDATA_GET(this->_context, this->_physAdapterIndex, &data);
    if (ADL_OK != result) return speedRpm;

    const ADLSingleSensorData& fanRpm = data.sensors[PMLOG_FAN_RPM];
    if (fanRpm.supported)
        speedRpm.push_back(fanRpm.value);

    return speedRpm;
}

Overdrive8::~Overdrive8() {
    if (this->_context && this->_ADL2_MAIN_CONTROL_DESTROY)
        this->_ADL2_MAIN_CONTROL_DESTROY(this->_context);
}