#pragma once

#include <sstream>
#include <iomanip>

#include <windows.h>
#include <setupapi.h>
#include <initguid.h>
#include <devguid.h>
#include <devpkey.h>

#include <dxgi1_6.h>

enum class TypeOfGPU {
	UNKNOWN_GPU = 0,
	NVIDIA_GPU = 1,
	AMD_GPU = 2,
	INTEL_GPU = 3,
};

class GPU {
public:
	GPU(IDXGIAdapter* pDXGIAdapter);

	std::wstring GetDeviceName() const { return this->_deviceName; }
	std::wstring GetDeviceID() const { return this->_deviceID; }
	std::wstring GetRevision() const { return this->_rev; }
	std::wstring GetDriverVersion() const { return this->_driverVersion; }
	std::wstring GetDriverDate() const { return this->_driverDate; }
	uint32_t GetMemSize() const { return this->_memSize; }
	
	~GPU();

protected:
	TypeOfGPU _whoIsMyDaddy;
	std::wstring _deviceName, _deviceID, _rev, _directXMaxVersion, _driverVersion, _driverDate;
	uint32_t _memSize;
	bool _hasVulkan, _hasRayTracing;
	IDXGIAdapter* _pDXGIAdapter;

private:

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
};