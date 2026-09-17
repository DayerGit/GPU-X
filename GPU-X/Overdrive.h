#pragma once
#include <stdint.h>
#include <vector>

#include <windows.h>
#include "AMD/adl_common.h"

class Overdrive {
public:
	virtual bool LoadLib() = 0;

	virtual double GetCoreVoltage() = 0;
	virtual int32_t GetCoreTemp() = 0;

	virtual void GetDefaultAndBoostClockInfo(int32_t* defaultCoreClock, int32_t* boostCoreClock, double* defaultMemoryClock, double* boostMemoryClock) = 0;
	virtual void GetClockInfo(double* memory, int32_t* core) = 0;
	virtual std::vector<uint32_t> GetFanInfo() = 0;

	virtual ~Overdrive() = default;
};