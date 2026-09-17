#pragma once

#include "Overdrive.h"

class Overdrive5 : public Overdrive {
public:
	Overdrive5(HMODULE _hAModule, int _physAdapterIndex);

	bool LoadLib() override;

	double GetCoreVoltage() override;
	int32_t GetCoreTemp() override;

	void GetDefaultAndBoostClockInfo(int32_t* defaultCoreClock, int32_t* boostCoreClock, double* defaultMemoryClock, double* boostMemoryClock) override;
	void GetClockInfo(double* memory, int32_t* core) override;
	std::vector<uint32_t> GetFanInfo() override;

	~Overdrive5();

private:
	HMODULE _hAModule;
	int _physAdapterIndex;

	ADL_OVERDRIVE5_FANSPEED_GET _ADL_OVERDRIVE5_FANSPEED_GET;
	ADL_OVERDRIVE5_CURRENTACTIVITY_GET _ADL_OVERDRIVE5_CURRENTACTIVITY_GET;

	ADL_OVERDRIVE5_ODPARAMETERS_GET _ADL_OVERDRIVE5_ODPARAMETERS_GET;
	ADL_OVERDRIVE5_ODPERFORMANCELEVELS_GET _ADL_OVERDRIVE5_ODPERFORMANCELEVELS_GET;

	ADL_OVERDRIVE5_TEMPERATURE_GET _ADL_OVERDRIVE5_TEMPERATURE_GET;
};