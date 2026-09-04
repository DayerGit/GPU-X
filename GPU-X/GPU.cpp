#include <iostream>
#include <vector>

#include "GPU.h"


GPU::GPU(IDXGIAdapter* pDXGIAdapter): _pDXGIAdapter(pDXGIAdapter), _whoIsMyDaddy(TypeOfGPU::UNKNOWN_GPU), _hasVulkan(false), _hasRayTracing(false) {
    if (!pDXGIAdapter) return;

    IDXGIAdapter1* pAdapter1 = nullptr;
    HRESULT hr = pDXGIAdapter->QueryInterface(IID_PPV_ARGS(&pAdapter1));

    if (SUCCEEDED(hr) && pAdapter1) {
        DXGI_ADAPTER_DESC1 desc1 = {};
        if (SUCCEEDED(pAdapter1->GetDesc1(&desc1))) 
            this->_FillFromDesc(desc1);

        pAdapter1->Release();
    }
    else {
        DXGI_ADAPTER_DESC desc = {};
        if (SUCCEEDED(pDXGIAdapter->GetDesc(&desc))) 
            this->_FillFromDesc(desc);
    }

    this->_FetchDriverInfo();
}

void GPU::_FetchDriverInfo() {
    HDEVINFO hDevInfo = SetupDiGetClassDevsW(&GUID_DEVCLASS_DISPLAY, nullptr, nullptr, DIGCF_PRESENT);

    if (hDevInfo == INVALID_HANDLE_VALUE) return;

    struct DevInfoListGuard {
        HDEVINFO handle;
        ~DevInfoListGuard() {
            if (handle != INVALID_HANDLE_VALUE) 
                SetupDiDestroyDeviceInfoList(handle);
        }
    } guard = { hDevInfo };

    SP_DEVINFO_DATA devInfoData = {};
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &devInfoData); i++) {
        const std::wstring deviceName = this->_GetDevicePropertyString(hDevInfo, &devInfoData, DEVPKEY_Device_DeviceDesc, DEVPKEY_Device_FriendlyName);

        if (deviceName != this->_deviceName) continue;

        this->_driverVersion = this->_GetDevicePropertyString(hDevInfo, &devInfoData, DEVPKEY_Device_DriverVersion, {});
        this->_driverDate = this->_GetDriverDate(hDevInfo, &devInfoData);
        break;
    }
}

std::wstring GPU::_GetDevicePropertyString(HDEVINFO hDevInfo, PSP_DEVINFO_DATA pDevInfoData, const DEVPROPKEY& key1, const DEVPROPKEY& key2) {
    const DEVPROPKEY* keys[] = { &key1, &key2 };
    const size_t count = (key2.fmtid.Data1 == 0 && key2.pid == 0) ? 1 : 2;

    for (size_t i = 0; i < count; i++) {
        DEVPROPTYPE propType = 0;
        DWORD requiredSize = 0;

        SetupDiGetDevicePropertyW(hDevInfo, pDevInfoData, keys[i], &propType, nullptr, 0, &requiredSize, 0);

        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER || requiredSize == 0) continue;

        std::vector<BYTE> buffer(requiredSize);
        if (SetupDiGetDevicePropertyW(hDevInfo, pDevInfoData, keys[i], &propType, buffer.data(), requiredSize, nullptr, 0))
            return reinterpret_cast<const wchar_t*>(buffer.data());
    }

    return L"";
}

std::wstring GPU::_GetDriverDate(HDEVINFO hDevInfo, PSP_DEVINFO_DATA pDevInfoData) {
    DEVPROPTYPE propType = 0;
    DWORD requiredSize = 0;

    SetupDiGetDevicePropertyW(hDevInfo, pDevInfoData, &DEVPKEY_Device_DriverDate, &propType, nullptr, 0, &requiredSize, 0);

    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER || requiredSize != sizeof(FILETIME)) return L"";

    FILETIME ft = {};
    if (!SetupDiGetDevicePropertyW(hDevInfo, pDevInfoData, &DEVPKEY_Device_DriverDate, &propType, reinterpret_cast<PBYTE>(&ft), requiredSize, nullptr, 0)) return L"";

    SYSTEMTIME st = {};
    if (!FileTimeToSystemTime(&ft, &st)) return L"";

    std::wstringstream ss;
    ss << std::setfill(L'0') << std::setw(2) << st.wDay << L'.' << std::setw(2) << st.wMonth << L'.' << std::setw(4) << st.wYear;
    return ss.str();
}

GPU::~GPU() {
	if (this->_pDXGIAdapter)
		this->_pDXGIAdapter->Release();
}