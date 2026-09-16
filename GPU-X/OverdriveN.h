#pragma once

#include "Overdrive.h"

class OverdriveN : Overdrive {
public:
	double GetCoreVoltage() override;
	int32_t GetCoreTemp() override;

	void GetDefaultAndBoostClockInfo(void* defaultClock, void* boostClock) override;
	void GetClockInfo(void* memory, void* core) override;
	std::vector<int32_t> GetFanInfo() override;
};