#include <iostream>
#include <vector>

#include "GPU.h"


GPU::GPU(IDXGIAdapter* pDXGIAdapter, int index): _pDXGIAdapter(pDXGIAdapter), _whoIsMyDaddy(TypeOfGPU::UNKNOWN_GPU), 
                                                 _hasVulkan(false), _hasRayTracing(false), _adapterIndex(index) {
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

    this->_directXMaxVersion = this->_GetDeviceD3DMaxVersion();
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

std::wstring GPU::_DXEnumToString(D3D_FEATURE_LEVEL level) {
    switch (level) {
    case D3D_FEATURE_LEVEL_12_2: return L"12 (12_2)";
    case D3D_FEATURE_LEVEL_12_1: return L"12 (12_1)";
    case D3D_FEATURE_LEVEL_12_0: return L"12 (12_0)";
    case D3D_FEATURE_LEVEL_11_1: return L"11 (11_1)";
    case D3D_FEATURE_LEVEL_11_0: return L"11 (11_0)";
    case D3D_FEATURE_LEVEL_10_1: return L"10 (10_1)";
    case D3D_FEATURE_LEVEL_10_0: return L"10 (10_0)";
    case D3D_FEATURE_LEVEL_9_3: return L"9 (9_3)";
    case D3D_FEATURE_LEVEL_9_2: return L"9 (9_2)";
    case D3D_FEATURE_LEVEL_9_1: return L"9 (9_1)";
    default: return L"UNKNOWN";
    }
}

std::wstring GPU::_GetDeviceD3DMaxVersion() {
    HMODULE hModule;

    hModule = LoadLibraryA("d3d12.dll");
    if (hModule) {
        ID3D12Device* pD3D12Device = nullptr;
        D3D12CreateDevice_t d3d12CreateDevice = (D3D12CreateDevice_t)GetProcAddress(hModule, "D3D12CreateDevice");

        if (d3d12CreateDevice) {
            for (int i = 0; levelsToCheck[i] != D3D_FEATURE_LEVEL_11_1; i++) {
                HRESULT hRes = d3d12CreateDevice(this->_pDXGIAdapter, levelsToCheck[i], IID_PPV_ARGS(&pD3D12Device));

                if (SUCCEEDED(hRes)) {

                    D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};

                    pD3D12Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5));

                    this->_hasRayTracing = options5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;

                    pD3D12Device->Release();
                    return this->_DXEnumToString(levelsToCheck[i]);
                }
            }
        }
    }

    hModule = LoadLibraryA("d3d11.dll");
    if (hModule) {
        ID3D11Device* pD3D11Device = nullptr;
        D3D_FEATURE_LEVEL maxSupportedLevel;
        D3D11CreateDevice_t d3d11CreateDevice = (D3D11CreateDevice_t)GetProcAddress(hModule, "D3D11CreateDevice");
 
        if (d3d11CreateDevice) {
            HRESULT hRes = d3d11CreateDevice(this->_pDXGIAdapter, D3D_DRIVER_TYPE_UNKNOWN, 0, 0, levelsToCheck, 
                                             ARRAYSIZE(levelsToCheck), D3D11_SDK_VERSION, &pD3D11Device, &maxSupportedLevel, 0);

            if (SUCCEEDED(hRes)) {
                pD3D11Device->Release();
                return this->_DXEnumToString(maxSupportedLevel);
            }
        }
    }

    hModule = LoadLibraryA("d3d10_1.dll");
    if (hModule) {
        ID3D10Device1* pD3D10Device1 = nullptr;
        D3D10CreateDevice1_t d3d10CreateDevice1 = (D3D10CreateDevice1_t)GetProcAddress(hModule, "D3D10CreateDevice1");

        if (d3d10CreateDevice1) {
            HRESULT hRes = d3d10CreateDevice1(this->_pDXGIAdapter, D3D10_DRIVER_TYPE_HARDWARE, NULL, 0, D3D10_FEATURE_LEVEL_10_1, 
                                              D3D10_1_SDK_VERSION, &pD3D10Device1);

            if (SUCCEEDED(hRes)) {
                pD3D10Device1->Release();
                return this->_DXEnumToString(levelsToCheck[5]);
            }
        }
    }

    hModule = LoadLibraryA("d3d10.dll");
    if (hModule) {
        ID3D10Device* pD3D10Device = nullptr;
        D3D10CreateDevice_t d3d10CreateDevice = (D3D10CreateDevice_t)GetProcAddress(hModule, "D3D10CreateDevice");

        if (d3d10CreateDevice) {
            HRESULT hRes = d3d10CreateDevice(this->_pDXGIAdapter, D3D10_DRIVER_TYPE_HARDWARE, NULL, 0, D3D10_SDK_VERSION, &pD3D10Device);

            if (SUCCEEDED(hRes)) {
                pD3D10Device->Release();
                return this->_DXEnumToString(levelsToCheck[6]);
            }
        }
    }

    hModule = LoadLibraryA("d3d9.dll");
    if (hModule) {
        D3D9Create_t d3d9Create = (D3D9Create_t)GetProcAddress(hModule, "Direct3DCreate9");

        if (d3d9Create) {
            IDirect3D9* pD3D9 = d3d9Create(D3D_SDK_VERSION);
            if (pD3D9) {
                D3DCAPS9 caps;
                HRESULT hRes = pD3D9->GetDeviceCaps(this->_adapterIndex, D3DDEVTYPE_HAL, &caps);
                if (SUCCEEDED(hRes)) {
                    DWORD majorShader = (caps.PixelShaderVersion & 0x0000FF00) >> 8;
                    DWORD minorShader = (caps.PixelShaderVersion & 0x000000FF);

                    D3D_FEATURE_LEVEL standardLevel = (D3D_FEATURE_LEVEL)0;

                    if (majorShader >= 3) standardLevel = D3D_FEATURE_LEVEL_9_3;
                    else if (majorShader == 2) {
                        if ((caps.PS20Caps.NumInstructionSlots >= 512) || (caps.PS20Caps.Caps & D3DPS20CAPS_DYNAMICFLOWCONTROL)) {
                            standardLevel = D3D_FEATURE_LEVEL_9_2;
                        }
                        else {
                            standardLevel = D3D_FEATURE_LEVEL_9_1;
                        }
                    }

                    return this->_DXEnumToString(standardLevel);
                }
            }
        }
    }

    return L"UNKNOWN";
}

GPU::~GPU() {
	if (this->_pDXGIAdapter)
		this->_pDXGIAdapter->Release();
}