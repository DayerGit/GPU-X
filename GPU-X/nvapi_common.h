#pragma once

#include "NVIDIA/nvapi.h"

using NvAPI_QueryInterface_t = void* (__cdecl*)(unsigned int);

#define NvAPI_Initialize_ID 0x0150E828
using NvAPI_Initialize_t = NvAPI_Status(__cdecl*)();

#define NvAPI_EnumPhysicalGPUs_ID 0xE5AC921F
using NvAPI_EnumPhysicalGPUs_t = NvAPI_Status(__cdecl*)(NvPhysicalGpuHandle nvGPUHandle[NVAPI_MAX_PHYSICAL_GPUS], NvU32* pGpuCount);

#define NvAPI_GPU_GetAllClockFrequencies_ID 0xDCB616C3
using NvAPI_GPU_GetAllClockFrequencies_t = NvAPI_Status(__cdecl*)(int hPhysicalGpu, void* pClkFreqs);

#define NvAPI_GPU_GetPstates20_ID 0x6FF81213
using NvAPI_GPU_GetPstates20_t = NvAPI_Status(__cdecl*)(int hPhysicalGpu, void* pPstatesInfo);

#define NvAPI_GPU_ClientPowerTopologyGetStatus_ID 0xEDCF624E
using NvAPI_GPU_ClientPowerTopologyGetStatus_t = NvAPI_Status(__cdecl*)(int hPhysicalGpu, void* pPowerTopologyStatus);

#define NvAPI_GPU_GetThermalSettings_ID 0xE3640A56
using NvAPI_GPU_GetThermalSettings_t = NvAPI_Status(__cdecl*)(int hPhysicalGpu, void* pThermalSettings);

#define NvAPI_GPU_ClientVoltRailsGetStatus_ID 0x465F9BCF
using NvAPI_GPU_ClientVoltRailsGetStatus_t = NvAPI_Status(__cdecl*)(int hPhysicalGpu, void* pVoltRailsStatus);

#define NvAPI_GPU_GetVbiosVersionString_ID 0xA561FD7D
using NvAPI_GPU_GetVbiosVersionString_t = NvAPI_Status(__cdecl*)(NvPhysicalGpuHandle hPhysicalGpu, NvAPI_ShortString szBiosRevision);

#define NvAPI_GPU_GetAdapterIdFromPhysicalGpu_ID 0x0FF07FDE
using NvAPI_GPU_GetAdapterIdFromPhysicalGpu_t = NvAPI_Status(__cdecl*)(NvPhysicalGpuHandle hPhysicalGpu, void* pOSAdapterId);

#define NvAPI_GPU_GetBusType_ID 0x1BB18724
using NvAPI_GPU_GetBusType_t = NvAPI_Status(__cdecl*)(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_BUS_TYPE* pBusType);

#define NvAPI_GPU_GetCurrentPCIEDownstreamWidth_ID 0xD048C3B1
using NvAPI_GPU_GetCurrentPCIEDownstreamWidth_t = NvAPI_Status(__cdecl*)(NvPhysicalGpuHandle hPhysicalGpu, NvU32* pWidth);

#define NvAPI_Unload_ID 0xD22BDD7E
using NvAPI_Unload_t = NvAPI_Status(__cdecl*)();

typedef struct {
	NvU32 unknown0;
	NvU32 unknown1;
	NvU32 unknown2;
	NvU32 unknown3;
	NvU32 unknown4;
	NvU32 unknown5;
	NvU32 unknown6;
	NvU32 unknown7;
} NV_PCIE_INFO_UNKNOWN;

typedef struct {
	NvU32 version;
	NV_PCIE_INFO_UNKNOWN info[5];
} NV_PCIE_INFO;

#define NV_PCIE_INFO_VER MAKE_NVAPI_VERSION(NV_PCIE_INFO, 2)

#define NvAPI_GPU_GetPCIEInfo_ID 0xE3795199
using NvAPI_GPU_GetPCIEInfo_t = NvAPI_Status(__cdecl*)(NvPhysicalGpuHandle hPhysicalGpu, NV_PCIE_INFO* pPciInfo);

#define NVAPI_MAX_FAN_COOLERS_STATUS_ITEMS  32

#pragma pack(push, 8)

struct NV_FAN_COOLERS_STATUS_ITEM {
	NvU32 coolerId;
	NvU32 currentRpm;
	NvU32 currentMinLevel;
	NvU32 currentMaxLevel;
	NvU32 currentLevel;
	NvU32 reserved[8];
};

struct NV_GPU_CLIENT_FAN_COOLERS_STATUS {
	NvU32 version;
	NvU32 count;
	NvU64 reserved1;
	NvU64 reserved2;
	NvU64 reserved3;
	NvU64 reserved4;
	NV_FAN_COOLERS_STATUS_ITEM items[NVAPI_MAX_FAN_COOLERS_STATUS_ITEMS];
};

#pragma pack(pop)

#define NV_GPU_CLIENT_FAN_COOLERS_STATUS_VER  MAKE_NVAPI_VERSION(NV_GPU_CLIENT_FAN_COOLERS_STATUS, 1)

#define NvAPI_ClientFanCoolersGetStatus_ID   0x35AED5E8
using NvAPI_ClientFanCoolersGetStatus_t = NvAPI_Status(__cdecl*)(NvPhysicalGpuHandle, NV_GPU_CLIENT_FAN_COOLERS_STATUS*);


typedef enum {
	NV_RAM_UNKNOWN = 0,
	NV_RAM_SDRAM = 1,  
	NV_RAM_DDR1 = 2,   
	NV_RAM_DDR2 = 3,   
	NV_RAM_GDDR2 = 4,  
	NV_RAM_GDDR3 = 5,  
	NV_RAM_GDDR4 = 6,  
	NV_RAM_DDR3 = 7,   
	NV_RAM_GDDR5 = 8,  
	NV_RAM_LPDDR2 = 9, 
	NV_RAM_GDDR5X = 10,
	NV_RAM_HBM1 = 11,  
	NV_RAM_HBM2 = 12,  
	NV_RAM_HBM2E = 13, 
	NV_RAM_GDDR6 = 14, 
	NV_RAM_GDDR6X = 15,
	NV_RAM_GDDR7 = 16, 
	NV_RAM_HBM3 = 17,  
	NV_RAM_HBM3E = 18  
} NV_GPU_RAM_TYPE;



#define NvAPI_GPU_GetRamType_ID 0x57F7CAAC
using NvAPI_GPU_GetRamType_t = NvAPI_Status(__cdecl*)(NvPhysicalGpuHandle, NvU32*);
