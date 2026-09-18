#pragma once

#include "Overdrive.h"

class Overdrive7 : public Overdrive {
public:
	Overdrive7(HMODULE _hAModule, int _physAdapterIndex);

	bool LoadLib() override;

	double GetCoreVoltage() override;
	int32_t GetCoreTemp() override;

	void GetDefaultAndBoostClockInfo(int32_t* defaultCoreClock, int32_t* boostCoreClock, double* defaultMemoryClock, double* boostMemoryClock) override;
	void GetClockInfo(double* memory, int32_t* core) override;
	std::vector<uint32_t> GetFanInfo() override;

	~Overdrive7();

private:
    HMODULE _hAModule;
    int _physAdapterIndex;
    ADL_CONTEXT_HANDLE _context;

    ADL2_MAIN_CONTROL_CREATE _ADL2_MAIN_CONTROL_CREATE;
    ADL2_MAIN_CONTROL_DESTROY _ADL2_MAIN_CONTROL_DESTROY;
    ADL2_OVERDRIVE_CAPS _ADL2_OVERDRIVE_CAPS;

    ADL2_OVERDRIVEN_CAPABILITIES_GET _ADL2_OVERDRIVEN_CAPABILITIES_GET;
    ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET _ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET;
    ADL2_OVERDRIVEN_FANCONTROL_GET _ADL2_OVERDRIVEN_FANCONTROL_GET;
    ADL2_OVERDRIVEN_TEMPERATURE_GET _ADL2_OVERDRIVEN_TEMPERATURE_GET;
    ADL2_OVERDRIVEN_SYSTEMCLOCKS_GET _ADL2_OVERDRIVEN_SYSTEMCLOCKS_GET;
    ADL2_OVERDRIVEN_MEMORYCLOCKS_GET _ADL2_OVERDRIVEN_MEMORYCLOCKS_GET;
};