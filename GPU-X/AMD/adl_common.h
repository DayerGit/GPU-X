#pragma once

#include "AMD/adl_sdk.h"

typedef int(__cdecl* ADL_MAIN_CONTROL_CREATE) (ADL_MAIN_MALLOC_CALLBACK callback, int iEnumConnectedAdapters);
typedef int(__cdecl* ADL_MAIN_CONTROL_DESTROY) (void);

typedef int(__cdecl* ADL_ADAPTER_NUMBEROFADAPTERS_GET) (int* lpNumAdapters);
typedef int(__cdecl* ADL_ADAPTER_ADAPTERINFO_GET) (LPAdapterInfo lpInfo, int iInputSize);
typedef int(__cdecl* ADL_ADAPTER_ACTIVE_GET) (int iAdapterIndex, int* lpStatus);
typedef int(__cdecl* ADL_ADAPTER_VIDEOBIOSINFO_GET) (int iAdapterIndex, ADLBiosInfo* lpBiosInfo);
typedef int(__cdecl* ADL_ADAPTER_MEMORYINFO_GET) (int iAdapterIndex, ADLMemoryInfo* lpMemoryInfo);

// ---------------------------------------------------------------------------
// Overdrive 5
// ---------------------------------------------------------------------------
typedef int(__cdecl* ADL_OVERDRIVE5_FANSPEED_GET) (int iAdapterIndex, int iThermalControllerIndex, ADLFanSpeedValue* lpFanSpeedValue);
typedef int(__cdecl* ADL_OVERDRIVE5_CURRENTACTIVITY_GET) (int iAdapterIndex, ADLPMActivity* lpActivity);
typedef int(__cdecl* ADL_OVERDRIVE5_ODPARAMETERS_GET) (int iAdapterIndex, ADLODParameters* lpOdParameters);
typedef int(__cdecl* ADL_OVERDRIVE5_ODPERFORMANCELEVELS_GET)(int iAdapterIndex, int iDefault, ADLODPerformanceLevels* lpOdPerformanceLevels);
typedef int(__cdecl* ADL_OVERDRIVE5_TEMPERATURE_GET) (int iAdapterIndex, int iThermalControllerIndex, ADLTemperature* lpTemperature);

// ---------------------------------------------------------------------------
// Overdrive 6
// ---------------------------------------------------------------------------
typedef int(__cdecl* ADL_OVERDRIVE6_FANSPEED_GET) (int iAdapterIndex, ADLOD6FanSpeedInfo* lpFanSpeedInfo);
typedef int(__cdecl* ADL_OVERDRIVE6_CURRENTSTATUS_GET) (int iAdapterIndex, ADLOD6CurrentStatus* lpCurrentStatus);
typedef int(__cdecl* ADL_OVERDRIVE6_CAPABILITIES_GET) (int iAdapterIndex, ADLOD6Capabilities* lpODCapabilities);
typedef int(__cdecl* ADL_OVERDRIVE6_STATEINFO_GET) (int iAdapterIndex, int iStateType, ADLOD6StateInfo* lpStateInfo);
typedef int(__cdecl* ADL_OVERDRIVE6_TEMPERATURE_GET) (int iAdapterIndex, int* lpTemperature);
typedef int (*ADL_OVERDRIVE6_VOLTAGECONTROL_GET)(int iAdapterIndex, int* cur, int* def);

// ---------------------------------------------------------------------------
// OverdriveN
// ---------------------------------------------------------------------------
typedef int(__cdecl* ADL2_MAIN_CONTROL_CREATE) (ADL_MAIN_MALLOC_CALLBACK callback, int iEnumConnectedAdapters, ADL_CONTEXT_HANDLE* context);
typedef int(__cdecl* ADL2_MAIN_CONTROL_DESTROY) (ADL_CONTEXT_HANDLE context);

typedef int(__cdecl* ADL2_OVERDRIVE_CAPS) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* iSupported, int* iEnabled, int* iVersion);
typedef int(__cdecl* ADL2_OVERDRIVEN_CAPABILITIES_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODNCapabilities* lpODCapabilities);
typedef int(__cdecl* ADL2_OVERDRIVEN_CAPABILITIESX2_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODNCapabilitiesX2* lpODCapabilities);
typedef int(__cdecl* ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODNPerformanceStatus* lpODPerformanceStatus);
typedef int(__cdecl* ADL2_OVERDRIVEN_FANCONTROL_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODNFanControl* lpODFanControl);
typedef int(__cdecl* ADL2_OVERDRIVEN_TEMPERATURE_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iTemperatureType, int* iTemperature);
typedef int(__cdecl* ADL2_OVERDRIVEN_SYSTEMCLOCKS_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODNPerformanceLevels* lpODPerformanceLevels);
typedef int(__cdecl* ADL2_OVERDRIVEN_MEMORYCLOCKS_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODNPerformanceLevels* lpODPerformanceLevels);
typedef int(__cdecl* ADL2_OVERDRIVEN_SYSTEMCLOCKSX2_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODNPerformanceLevelsX2* lpODPerformanceLevels);
typedef int(__cdecl* ADL2_OVERDRIVEN_MEMORYCLOCKSX2_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODNPerformanceLevelsX2* lpODPerformanceLevels);
typedef int(__cdecl* ADL2_OVERDRIVEN_POWERLIMIT_GET)(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODNPowerLimitSetting* lpODPowerLimit);

// ---------------------------------------------------------------------------
// Overdrive8
// ---------------------------------------------------------------------------
typedef int(__cdecl* ADL2_OVERDRIVE8_INIT_SETTING_GET)(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLOD8InitSetting* lpInitSetting);
typedef int(__cdecl* ADL2_OVERDRIVE8_CURRENT_SETTING_GET)(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLOD8CurrentSetting* lpCurrentSetting);

typedef int(__cdecl* ADL2_OVERDRIVE8_INIT_SETTINGX2_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* lpOverdrive8Capabilities, 
    int* lpNumberOfFeatures, ADLOD8SingleInitSetting** lppInitSettingList);

typedef int(__cdecl* ADL2_OVERDRIVE8_CURRENT_SETTINGX2_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, 
    int* lpNumberOfFeatures, int** lppCurrentSettingList);

typedef int(__cdecl* ADL2_OVERDRIVE8_CURRENT_SETTINGX3_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, 
    int* lpFeatureNotAdjustableBits, int* lpNumberOfSettings, int** lppCurrentSettingList, int iOption);

typedef int(__cdecl* ADL2_NEW_QUERYPMLOGDATA_GET)(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLPMLogDataOutput* lpDataOutput);