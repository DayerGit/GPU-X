#include "Overdrive6.h"

Overdrive6::Overdrive6(HMODULE _hAModule, int _physAdapterIndex) {
	this->_hAModule = _hAModule;
	this->_physAdapterIndex = _physAdapterIndex;
}

bool Overdrive6::LoadLib() {
	this->_ADL_OVERDRIVE6_FANSPEED_GET =
		(ADL_OVERDRIVE6_FANSPEED_GET)GetProcAddress(this->_hAModule, "ADL_Overdrive6_FanSpeed_Get");

	if (!this->_ADL_OVERDRIVE6_FANSPEED_GET) return false;

	this->_ADL_OVERDRIVE6_CURRENTSTATUS_GET =
		(ADL_OVERDRIVE6_CURRENTSTATUS_GET)GetProcAddress(this->_hAModule, "ADL_Overdrive6_CurrentStatus_Get");

	if (!this->_ADL_OVERDRIVE6_CURRENTSTATUS_GET) return false;

	this->_ADL_OVERDRIVE6_CAPABILITIES_GET =
		(ADL_OVERDRIVE6_CAPABILITIES_GET)GetProcAddress(this->_hAModule, "ADL_Overdrive6_Capabilities_Get");

	if (!this->_ADL_OVERDRIVE6_CAPABILITIES_GET) return false;

	this->_ADL_OVERDRIVE6_STATEINFO_GET =
		(ADL_OVERDRIVE6_STATEINFO_GET)GetProcAddress(this->_hAModule, "ADL_Overdrive6_StateInfo_Get");

	if (!this->_ADL_OVERDRIVE6_STATEINFO_GET) return false;

	this->_ADL_OVERDRIVE6_TEMPERATURE_GET =
		(ADL_OVERDRIVE6_TEMPERATURE_GET)GetProcAddress(this->_hAModule, "ADL_Overdrive6_Temperature_Get");

	if (!this->_ADL_OVERDRIVE6_TEMPERATURE_GET) return false;

	this->_ADL_OVERDRIVE6_VOLTAGECONTROL_GET =
		(ADL_OVERDRIVE6_VOLTAGECONTROL_GET)GetProcAddress(this->_hAModule, "ADL_Overdrive6_VoltageControl_Get");

	if (!this->_ADL_OVERDRIVE6_VOLTAGECONTROL_GET) return false;

	return true;
}

double Overdrive6::GetCoreVoltage() {
	ADLOD6Capabilities od6Caps = { 0 };
	this->_ADL_OVERDRIVE6_CAPABILITIES_GET(this->_physAdapterIndex, &od6Caps);

	int currentValue = 0;
	int defaultValue = 0;

	this->_ADL_OVERDRIVE6_VOLTAGECONTROL_GET(this->_physAdapterIndex, &currentValue, &defaultValue);

	return currentValue / 1000.0;
}
int32_t Overdrive6::GetCoreTemp() {
	int temperature = 0;

	this->_ADL_OVERDRIVE6_TEMPERATURE_GET(this->_physAdapterIndex, &temperature);
	return temperature / 1000;
}

void Overdrive6::GetDefaultAndBoostClockInfo(int32_t* defaultCoreClock, int32_t* boostCoreClock, double* defaultMemoryClock, double* boostMemoryClock) {
	ADLOD6Capabilities od6Caps = { 0 };

	int result = this->_ADL_OVERDRIVE6_CAPABILITIES_GET(this->_physAdapterIndex, &od6Caps);
	if (result == ADL_OK) {
		auto totalLevels = od6Caps.iNumberOfPerformanceLevels;
		int structSize = sizeof(ADLOD6StateInfo) + sizeof(ADLOD6PerformanceLevel) * (totalLevels - 1);
		ADLOD6StateInfo* stateInfo = (ADLOD6StateInfo*)calloc(1, structSize);
		if (!stateInfo) return;
		stateInfo->iNumberOfPerformanceLevels = totalLevels;

		this->_ADL_OVERDRIVE6_STATEINFO_GET(this->_physAdapterIndex, 1, stateInfo);

		*defaultCoreClock = stateInfo->aLevels[0].iEngineClock / 100.0;
		*boostCoreClock = stateInfo->aLevels[totalLevels - 1].iEngineClock / 100.0;

		*defaultMemoryClock = stateInfo->aLevels[0].iMemoryClock / 100.0;
		*boostMemoryClock = stateInfo->aLevels[totalLevels - 1].iMemoryClock / 100.0;

		free(stateInfo);
	}
}

void Overdrive6::GetClockInfo(double* memory, int32_t* core) {
	ADLOD6CurrentStatus adlodCS = { 0 };
	int result = this->_ADL_OVERDRIVE6_CURRENTSTATUS_GET(this->_physAdapterIndex, &adlodCS);
	if (ADL_OK == result) {
		*memory = adlodCS.iMemoryClock / 100.0;
		*core = adlodCS.iEngineClock / 100.0;
	}
}
std::vector<uint32_t> Overdrive6::GetFanInfo() {
	ADLOD6FanSpeedInfo adlod6FSI = { 0 };
	adlod6FSI.iSpeedType = ADL_OD6_FANSPEED_TYPE_RPM;

	int result = this->_ADL_OVERDRIVE6_FANSPEED_GET(this->_physAdapterIndex, &adlod6FSI);
	
	std::vector<uint32_t> speedRpm;
	if (ADL_OK == result)
		speedRpm.push_back(adlod6FSI.iFanSpeedRPM);

	return speedRpm;
}

Overdrive6::~Overdrive6() {}