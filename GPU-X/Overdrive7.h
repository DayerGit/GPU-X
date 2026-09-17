#pragma once

#include "Overdrive.h"

class Overdrive7 : public Overdrive {
public:
	Overdrive7(HMODULE _hAModule, int _physAdapterIndex) {};

	bool LoadLib() override { return false; };

	double GetCoreVoltage() override { return 0; };
	int32_t GetCoreTemp() override { return 0; };

	void GetDefaultAndBoostClockInfo(int32_t* defaultCoreClock, int32_t* boostCoreClock, double* defaultMemoryClock, double* boostMemoryClock) override { return; };
	void GetClockInfo(double* memory, int32_t* core) override { return; };
	std::vector<uint32_t> GetFanInfo() override { return std::vector<uint32_t>(); };

	~Overdrive7() {};
};