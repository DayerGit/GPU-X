#pragma once
#include <stdint.h>
#include <vector>

class Overdrive {
public:
	virtual double GetCoreVoltage() = 0;
	virtual int32_t GetCoreTemp() = 0;

	virtual void GetDefaultAndBoostClockInfo(void* defaultClock, void* boostClock) = 0;
	virtual void GetClockInfo(void* memory, void* core) = 0;
	virtual std::vector<int32_t> GetFanInfo() = 0;
};