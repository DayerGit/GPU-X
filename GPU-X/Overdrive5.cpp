#include "Overdrive5.h"

Overdrive5::Overdrive5(HMODULE _hAModule, int _physAdapterIndex) {
	this->_hAModule = _hAModule;
	this->_physAdapterIndex = _physAdapterIndex;
}

bool Overdrive5::LoadLib() {
	this->_ADL_OVERDRIVE5_CURRENTACTIVITY_GET =
		(ADL_OVERDRIVE5_CURRENTACTIVITY_GET)GetProcAddress(_hAModule, "ADL_Overdrive5_CurrentActivity_Get");

	if (!this->_ADL_OVERDRIVE5_CURRENTACTIVITY_GET) return false;

	this->_ADL_OVERDRIVE5_ODPARAMETERS_GET =
		(ADL_OVERDRIVE5_ODPARAMETERS_GET)GetProcAddress(_hAModule, "ADL_Overdrive5_ODParameters_Get");

	if (!this->_ADL_OVERDRIVE5_ODPARAMETERS_GET) return false;

	this->_ADL_OVERDRIVE5_ODPERFORMANCELEVELS_GET =
		(ADL_OVERDRIVE5_ODPERFORMANCELEVELS_GET)GetProcAddress(_hAModule, "ADL_Overdrive5_ODPerformanceLevels_Get");

	if (!this->_ADL_OVERDRIVE5_ODPERFORMANCELEVELS_GET) return false;

	this->_ADL_OVERDRIVE5_TEMPERATURE_GET =
		(ADL_OVERDRIVE5_TEMPERATURE_GET)GetProcAddress(_hAModule, "ADL_Overdrive5_Temperature_Get");

	if (!this->_ADL_OVERDRIVE5_TEMPERATURE_GET) return false;

	this->_ADL_OVERDRIVE5_FANSPEED_GET =
		(ADL_OVERDRIVE5_FANSPEED_GET)GetProcAddress(_hAModule, "ADL_Overdrive5_FanSpeed_Get");

	if (!this->_ADL_OVERDRIVE5_FANSPEED_GET) return false;

	return true;
}

double Overdrive5::GetCoreVoltage() {
	ADLPMActivity adlPM = { 0 };
	adlPM.iSize = sizeof(adlPM);

	this->_ADL_OVERDRIVE5_CURRENTACTIVITY_GET(this->_physAdapterIndex, &adlPM);
	
	return adlPM.iVddc / 1000.0;
}
int32_t Overdrive5::GetCoreTemp() {
	ADLTemperature adlTemp = { 0 };
	adlTemp.iSize = sizeof(adlTemp);

	this->_ADL_OVERDRIVE5_TEMPERATURE_GET(this->_physAdapterIndex, 0, &adlTemp);

	return adlTemp.iTemperature / 1000;
}

void Overdrive5::GetDefaultAndBoostClockInfo(int32_t* defaultCoreClock, int32_t* boostCoreClock, double* defaultMemoryClock, double* boostMemoryClock) {
	ADLODParameters odParams = { 0 };
	odParams.iSize = sizeof(ADLODParameters);

	int result = this->_ADL_OVERDRIVE5_ODPARAMETERS_GET(this->_physAdapterIndex, &odParams);
	if (result == ADL_OK) {
		auto totalLevels = odParams.iNumberOfPerformanceLevels;
		int structureSize = sizeof(ADLODPerformanceLevels) + sizeof(ADLODPerformanceLevel) * (totalLevels - 1);
		ADLODPerformanceLevels* allocLevels = (ADLODPerformanceLevels*)calloc(1, structureSize);
		if (!allocLevels) return;
		allocLevels->iSize = structureSize;

		this->_ADL_OVERDRIVE5_ODPERFORMANCELEVELS_GET(this->_physAdapterIndex, 1, allocLevels);

		*defaultCoreClock = allocLevels->aLevels[0].iEngineClock / 100.0;
		*boostCoreClock = allocLevels->aLevels[totalLevels - 1].iEngineClock / 100.0;

		*defaultMemoryClock = allocLevels->aLevels[0].iMemoryClock / 100.0;
		*boostMemoryClock = allocLevels->aLevels[totalLevels - 1].iMemoryClock / 100.0;

		free(allocLevels);
	}
}

void Overdrive5::GetClockInfo(double* memory, int32_t* core) {
	ADLPMActivity adlPM = { 0 };
	adlPM.iSize = sizeof(adlPM);
	int result = this->_ADL_OVERDRIVE5_CURRENTACTIVITY_GET(this->_physAdapterIndex, &adlPM);
	if (ADL_OK == result) {
		*memory = adlPM.iMemoryClock / 100.0;
		*core = adlPM.iEngineClock / 100.0;
	}
}
std::vector<uint32_t> Overdrive5::GetFanInfo() {
	ADLFanSpeedValue adlFSV = { 0 };
	adlFSV.iSize = sizeof(adlFSV);
	adlFSV.iSpeedType = ADL_DL_FANCTRL_SPEED_TYPE_RPM;

	int result = this->_ADL_OVERDRIVE5_FANSPEED_GET(this->_physAdapterIndex, 0, &adlFSV);
	
	std::vector<uint32_t> speedRpm;
	if (ADL_OK == result)
		speedRpm.push_back(adlFSV.iFanSpeed);

	return speedRpm;
}

Overdrive5::~Overdrive5() {}