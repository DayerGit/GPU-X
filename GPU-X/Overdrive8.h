#pragma once

#include "Overdrive.h"

class Overdrive8 : public Overdrive {
public:
	Overdrive8(HMODULE _hAModule, int _physAdapterIndex);

	bool LoadLib() override;

	double GetCoreVoltage() override;
	int32_t GetCoreTemp() override;

	void GetDefaultAndBoostClockInfo(int32_t* defaultCoreClock, int32_t* boostCoreClock, double* defaultMemoryClock, double* boostMemoryClock) override;
	void GetClockInfo(double* memory, int32_t* core) override;
	std::vector<uint32_t> GetFanInfo() override;

	~Overdrive8();

private:
    HMODULE _hAModule;
    int _physAdapterIndex;
    ADL_CONTEXT_HANDLE _context;

    ADL2_MAIN_CONTROL_CREATE _ADL2_MAIN_CONTROL_CREATE;
    ADL2_MAIN_CONTROL_DESTROY _ADL2_MAIN_CONTROL_DESTROY;
    ADL2_OVERDRIVE_CAPS _ADL2_OVERDRIVE_CAPS;

    ADL2_OVERDRIVE8_INIT_SETTING_GET _ADL2_OVERDRIVE8_INIT_SETTING_GET;
    ADL2_OVERDRIVE8_CURRENT_SETTING_GET _ADL2_OVERDRIVE8_CURRENT_SETTING_GET;
    ADL2_NEW_QUERYPMLOGDATA_GET _ADL2_NEW_QUERYPMLOGDATA_GET;

    ADLOD8InitSetting _initSetting = { 0 };
    bool _initSettingValid;
};