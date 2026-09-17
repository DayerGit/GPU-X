#pragma once

#include "Overdrive.h"

class Overdrive6 : public Overdrive {
public:
	Overdrive6(HMODULE _hAModule, int _physAdapterIndex);

	bool LoadLib() override;

	double GetCoreVoltage() override;
	int32_t GetCoreTemp() override;

	void GetDefaultAndBoostClockInfo(int32_t* defaultCoreClock, int32_t* boostCoreClock, double* defaultMemoryClock, double* boostMemoryClock) override;
	void GetClockInfo(double* memory, int32_t* core) override;
	std::vector<uint32_t> GetFanInfo() override;

	~Overdrive6();

private:
	HMODULE _hAModule;
	int _physAdapterIndex;

	ADL_OVERDRIVE6_FANSPEED_GET _ADL_OVERDRIVE6_FANSPEED_GET;
	ADL_OVERDRIVE6_CURRENTSTATUS_GET _ADL_OVERDRIVE6_CURRENTSTATUS_GET;

	ADL_OVERDRIVE6_CAPABILITIES_GET _ADL_OVERDRIVE6_CAPABILITIES_GET;
	ADL_OVERDRIVE6_STATEINFO_GET _ADL_OVERDRIVE6_STATEINFO_GET;

	ADL_OVERDRIVE6_TEMPERATURE_GET _ADL_OVERDRIVE6_TEMPERATURE_GET;

	ADL_OVERDRIVE6_VOLTAGECONTROL_GET _ADL_OVERDRIVE6_VOLTAGECONTROL_GET;
};