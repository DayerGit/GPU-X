#pragma once

#include <sstream>
#include <iomanip>

#include <windows.h>
#include <setupapi.h>
#include <initguid.h>
#include <devguid.h>
#include <devpkey.h>
#include <cfgmgr32.h> 

#include <dxgi.h>
#include <d3d12.h>
#include <directml.h>
#include <d3d11.h>
#include <d3d10_1.h>
#include <d3d10.h>
#include <d3d9.h>

#include <vulkan/vulkan.h>

#include <CL\cl.h>

#ifndef D3DPS20CAPS_DYNAMICFLOWCONTROL
#define D3DPS20CAPS_DYNAMICFLOWCONTROL 0x00000001
#endif

#ifndef CL_DEVICE_LUID_KHR

#define CL_UUID_SIZE_KHR             16
#define CL_LUID_SIZE_KHR             8

#define CL_DEVICE_UUID_KHR           0x10A5
#define CL_DEVICE_LUID_VALID_KHR     0x10A6
#define CL_DEVICE_LUID_KHR           0x10A7

#define CL_DEVICE_LUID_VALID		 0x106C
#define CL_DEVICE_LUID				 0x106D
#define CL_LUID_SIZE                 8

#endif

#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001

using D3D12CreateDevice_t = HRESULT(__stdcall*)(IUnknown*, D3D_FEATURE_LEVEL, REFIID, void**);
using DMLCreateDevice_t = HRESULT(__stdcall*)(ID3D12Device*, DML_CREATE_DEVICE_FLAGS, REFIID, void**);
using D3D11CreateDevice_t = HRESULT(__stdcall*)(IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT, CONST D3D_FEATURE_LEVEL*, UINT, UINT,
												ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext**);

using D3D10CreateDevice1_t = HRESULT(__stdcall*)(IDXGIAdapter*, D3D10_DRIVER_TYPE, HMODULE, UINT, D3D10_FEATURE_LEVEL1, UINT, ID3D10Device1**);
using D3D10CreateDevice_t = HRESULT(__stdcall*)(IDXGIAdapter*, D3D10_DRIVER_TYPE, HMODULE, UINT, UINT, ID3D10Device**);

using D3D9Create_t = IDirect3D9* (__stdcall*)(UINT);

using clGetPlatformIDs_t = cl_int(__stdcall*)(cl_uint, cl_platform_id*, cl_uint*);
using clGetDeviceIDs_t = cl_int(__stdcall*)(cl_platform_id, cl_device_type, cl_uint, cl_device_id*, cl_uint*);
using clGetDeviceInfo_t = cl_int(__stdcall*)(cl_device_id, cl_device_info, size_t, void*, size_t*);

using PFNWGLCREATECONTEXTATTRIBSARBPROC = HGLRC(__stdcall*)(HDC hDC, HGLRC hShareContext, const int* attribList);

enum class TypeOfGPU {
	UNKNOWN_GPU = 0,
	NVIDIA_GPU = 1,
	AMD_GPU = 2,
	INTEL_GPU = 3,
};

class GPU {
public:
	GPU(IDXGIAdapter* pDXGIAdapter, LUID AdapterLUID, int index, VkInstance vkInstance);

	std::wstring GetDeviceName() const { return this->_deviceName; }
	std::wstring GetDeviceID() const { return this->_deviceID; }
	std::wstring GetRevision() const { return this->_rev; }
	std::wstring GetDriverVersion() const { return this->_driverVersion; }
	std::wstring GetDriverDate() const { return this->_driverDate; }
	std::wstring GetDXMaxVersion() const { return this->_directXMaxVersion; }

	bool GetHasRayTracing() const { return this->_hasRayTracing; }
	bool GetHasDirectCompute() const { return this->_hasDirectCompute; }
	bool GetHasDirectML() const { return this->_hasDirectML; }
	bool GetHasVulkan() const { return this->_hasVulkan; }
	bool GetHasOpenCL() const { return this->_hasOpenCL; }
	bool GetHasOGL4_6() const { return this->_hasOGL4_6; }
	bool GetHasResizableBAR() const { return this->_hasResizableBar; }

	uint32_t GetMemSize() const { return this->_memSize; }

	TypeOfGPU GetProducer() const { return this->_whoIsMyDaddy; }
	
	~GPU();

protected:
	TypeOfGPU _whoIsMyDaddy;
	std::wstring _deviceName, _deviceID, _rev, _directXMaxVersion, _driverVersion, _driverDate;
	uint32_t _memSize;
	bool _hasOpenCL, _hasDirectCompute, _hasDirectML, _hasVulkan, _hasRayTracing, _hasOGL4_6, _hasResizableBar;

	IDXGIAdapter* _pDXGIAdapter;
	LUID _adapterLUID;
	int _adapterIndexForD3D9;

	VkInstance _vkInstance;

private:

	D3D_FEATURE_LEVEL levelsToCheck[10] = {
		   D3D_FEATURE_LEVEL_12_2,
		   D3D_FEATURE_LEVEL_12_1,
		   D3D_FEATURE_LEVEL_12_0,
		   D3D_FEATURE_LEVEL_11_1,
		   D3D_FEATURE_LEVEL_11_0,
		   D3D_FEATURE_LEVEL_10_1,
		   D3D_FEATURE_LEVEL_10_0,
		   D3D_FEATURE_LEVEL_9_3,
		   D3D_FEATURE_LEVEL_9_2,
		   D3D_FEATURE_LEVEL_9_1
	};

	template <typename DescT>
	void _FillFromDesc(const DescT& desc) {
		this->_deviceName = desc.Description;

		const uint16_t subVendorId = static_cast<uint16_t>(desc.SubSysId & 0xFFFF);
		const uint16_t subSystemId = static_cast<uint16_t>((desc.SubSysId >> 16) & 0xFFFF);

		{
			std::wstringstream ss;
			ss << std::uppercase << std::hex << std::setfill(L'0') << std::setw(4) << desc.VendorId << L' ' << std::setw(4) << desc.DeviceId << L" - " 
				<< std::setw(4) << subVendorId << L' ' << std::setw(4) << subSystemId;
			this->_deviceID = ss.str();
		}

		{
			std::wstringstream ss;
			ss << std::uppercase << std::hex << std::setfill(L'0') << std::setw(2) << desc.Revision;
			this->_rev = ss.str();
		}

		this->_memSize = static_cast<uint32_t>(desc.DedicatedVideoMemory / (1024 * 1024));
	}

	void _FetchDriverInfo();

	std::wstring _GetDevicePropertyString(HDEVINFO hDevInfo, PSP_DEVINFO_DATA pDevInfoData, const DEVPROPKEY& key1, const DEVPROPKEY& key2);
	std::wstring _GetDriverDate(HDEVINFO hDevInfo, PSP_DEVINFO_DATA pDevInfoData);

	std::wstring _GetDeviceD3DInfo();
	void _GetDMLInfo(ID3D12Device* pD3D12Device);
	std::wstring _DXEnumToString(D3D_FEATURE_LEVEL level);

	bool _CheckVulkan();
	bool _CheckOpenCL();
	bool _CheckOpenGL();

	void _CheckResizableBar(SP_DEVINFO_DATA devInfoData);
};