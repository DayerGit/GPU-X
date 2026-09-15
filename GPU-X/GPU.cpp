#include <iostream>
#include <vector>

#include "GPU.h"


GPU::GPU(IDXGIAdapter* pDXGIAdapter, LUID AdapterLUID, int index, VkInstance vkInstance): _pDXGIAdapter(pDXGIAdapter), 
                                                _whoIsMyDaddy(TypeOfGPU::UNKNOWN_GPU), _vkInstance(vkInstance),
                                                _hasOpenCL(false), _hasDirectCompute(false), _hasDirectML(false), 
                                                _hasVulkan(false), _hasRayTracing(false), _hasOGL4_6(false), _hasResizableBar(false),
                                                _adapterLUID(AdapterLUID), _adapterIndexForD3D9(index), _memSize(0)
{
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

    this->_directXMaxVersion = this->_GetDeviceD3DInfo();
    this->_hasVulkan = this->_CheckVulkan();
    this->_hasOpenCL = this->_CheckOpenCL();
    this->_hasOGL4_6 = this->_CheckOpenGL();
}

void GPU::_CheckResizableBar(SP_DEVINFO_DATA devInfoData) {
    LOG_CONF logConf = 0;
    if (CM_Get_First_Log_Conf(&logConf, devInfoData.DevInst, ALLOC_LOG_CONF) == CR_SUCCESS) {
        RES_DES resDes = 0;
        if (CM_Get_Next_Res_Des(&resDes, logConf, ResType_Mem, nullptr, 0) == CR_SUCCESS) {

            while (true) {
                MEM_RESOURCE memData = {};
                if (CM_Get_Res_Des_Data(resDes, &memData, sizeof(memData), 0) != CR_SUCCESS) break;

                ULONG64 barSize = memData.MEM_Header.MD_Count;

                if (barSize > 256 * 1024 * 1024) {
                    this->_hasResizableBar = true;
                    break;
                }

                RES_DES nextResDes = 0;
                if (CM_Get_Next_Res_Des(&nextResDes, resDes, ResType_Mem, nullptr, 0) != CR_SUCCESS) {
                    CM_Free_Res_Des_Handle(resDes);
                    break;
                }
                CM_Free_Res_Des_Handle(resDes);
                resDes = nextResDes;
            }
        }
        CM_Free_Log_Conf_Handle(logConf);
    }
}

void GPU::_FetchDriverInfo() {
    HDEVINFO hDevInfo = SetupDiGetClassDevsW(&GUID_DEVCLASS_DISPLAY, nullptr, nullptr, DIGCF_PRESENT);

    if (hDevInfo == INVALID_HANDLE_VALUE) return;

    SP_DEVINFO_DATA devInfoData = {};
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &devInfoData); i++) {
        const std::wstring deviceName = this->_GetDevicePropertyString(hDevInfo, &devInfoData, DEVPKEY_Device_DeviceDesc, DEVPKEY_Device_FriendlyName);

        if (deviceName != this->_deviceName) continue;

        this->_driverVersion = this->_GetDevicePropertyString(hDevInfo, &devInfoData, DEVPKEY_Device_DriverVersion, {});
        this->_driverDate = this->_GetDriverDate(hDevInfo, &devInfoData);

        this->_FillPCILocation(hDevInfo, &devInfoData);

        this->_CheckResizableBar(devInfoData);
        this->_hDevInfo = hDevInfo;
        this->_devInfoData = devInfoData;
        break;
    }
}

void GPU::_FillPCILocation(HDEVINFO hDevInfo, PSP_DEVINFO_DATA pDevInfoData) {
    DEVPROPTYPE propType = 0;
    DWORD requiredSize = 0;
    DWORD busNumber = MAXDWORD;
    DWORD address = MAXDWORD;

    if (SetupDiGetDevicePropertyW(hDevInfo, pDevInfoData, &DEVPKEY_Device_BusNumber, &propType, reinterpret_cast<PBYTE>(&busNumber), sizeof(busNumber), &requiredSize, 0) 
        && propType == DEVPROP_TYPE_UINT32) this->_pciBusNumber = busNumber;
    else return;

    if (SetupDiGetDevicePropertyW(hDevInfo, pDevInfoData, &DEVPKEY_Device_Address, &propType, reinterpret_cast<PBYTE>(&address), sizeof(address), &requiredSize, 0)
        && propType == DEVPROP_TYPE_UINT32) {
        this->_pciDeviceNumber = (address >> 16) & 0xFFFF;
        this->_pciFunctionNumber = address & 0xFFFF;
        this->_pciLocationValid = true;
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

void GPU::_GetDMLInfo(ID3D12Device* pD3D12Device) {
    HMODULE hDirectML = LoadLibraryA("directml.dll");
    if (hDirectML) {
        IDMLDevice* pDMLDevice = nullptr;

        DMLCreateDevice_t dmlCreateDevice = (DMLCreateDevice_t)GetProcAddress(hDirectML, "DMLCreateDevice");
        if (dmlCreateDevice) {
            HRESULT hRes = dmlCreateDevice(pD3D12Device, DML_CREATE_DEVICE_FLAG_NONE, IID_PPV_ARGS(&pDMLDevice));

            if (SUCCEEDED(hRes)) {
                this->_hasDirectML = true;
                pDMLDevice->Release();
            }
        }

        FreeLibrary(hDirectML);
    }
}

std::wstring GPU::_GetDeviceD3DInfo() {
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
                    this->_hasDirectCompute = true;

                    this->_GetDMLInfo(pD3D12Device);

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
                if (maxSupportedLevel >= D3D_FEATURE_LEVEL_11_0) this->_hasDirectCompute = true;
                else if (maxSupportedLevel >= D3D_FEATURE_LEVEL_10_0 && maxSupportedLevel <= D3D_FEATURE_LEVEL_10_1) {
                    D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS options = {};
                    pD3D11Device->CheckFeatureSupport(D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &options, sizeof(options));

                    this->_hasDirectCompute = options.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x;
                }
                else this->_hasDirectCompute = false;

                pD3D11Device->Release();
                return this->_DXEnumToString(maxSupportedLevel);
            }
        }
    }

    this->_hasDirectCompute = false;

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
                HRESULT hRes = pD3D9->GetDeviceCaps(this->_adapterIndexForD3D9, D3DDEVTYPE_HAL, &caps);
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

bool GPU::_CheckVulkan() {
    if (this->_vkInstance == VK_NULL_HANDLE) 
        return false;

    HMODULE hModule;

    hModule = LoadLibraryA("vulkan-1.dll");
    if (!hModule) return false;

    PFN_vkEnumeratePhysicalDevices pfnEnumeratePhysicalDevicesProcAddr = (PFN_vkEnumeratePhysicalDevices)GetProcAddress(hModule, 
                                                                            "vkEnumeratePhysicalDevices");
    PFN_vkGetPhysicalDeviceProperties pfnGetPhysicalDevicePropertiesProcAddr = (PFN_vkGetPhysicalDeviceProperties)GetProcAddress(hModule, 
                                                                                "vkGetPhysicalDeviceProperties");
    PFN_vkGetPhysicalDeviceProperties2 pfnGetPhysicalDeviceProperties2 = (PFN_vkGetPhysicalDeviceProperties2)GetProcAddress(hModule, 
                                                                            "vkGetPhysicalDeviceProperties2");
    if (!pfnGetPhysicalDeviceProperties2)
        pfnGetPhysicalDeviceProperties2 = (PFN_vkGetPhysicalDeviceProperties2)GetProcAddress(hModule, "vkGetPhysicalDeviceProperties2KHR");

    if (!(pfnEnumeratePhysicalDevicesProcAddr && pfnGetPhysicalDevicePropertiesProcAddr && pfnGetPhysicalDeviceProperties2)) return false;

    uint32_t deviceCount = 0;
    pfnEnumeratePhysicalDevicesProcAddr(this->_vkInstance, &deviceCount, nullptr);
    if (deviceCount == 0) return false;

    std::vector<VkPhysicalDevice> devices(deviceCount);
    pfnEnumeratePhysicalDevicesProcAddr(this->_vkInstance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        VkPhysicalDeviceProperties deviceProperties;
        pfnGetPhysicalDevicePropertiesProcAddr(device, &deviceProperties);

        VkPhysicalDeviceIDProperties deviceIDProperties{};
        deviceIDProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;
        deviceIDProperties.pNext = nullptr;

        VkPhysicalDeviceProperties2 deviceProperties2{};
        deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        deviceProperties2.pNext = &deviceIDProperties;

        pfnGetPhysicalDeviceProperties2(device, &deviceProperties2);

        if (deviceIDProperties.deviceLUIDValid) {
            if (!memcmp(&this->_adapterLUID, deviceIDProperties.deviceLUID, sizeof(LUID))) 
                return true;
        }
    }

    return false;
}

bool GPU::_CheckOpenCL() {
    HMODULE hModule;

    hModule = LoadLibraryA("OpenCL.dll");
    if (!hModule) return false;

    clGetPlatformIDs_t _clGetPlatformIDs = (clGetPlatformIDs_t)GetProcAddress(hModule, "clGetPlatformIDs");
    clGetDeviceIDs_t _clGetDeviceIDs = (clGetDeviceIDs_t)GetProcAddress(hModule, "clGetDeviceIDs");
    clGetDeviceInfo_t _clGetDeviceInfo = (clGetDeviceInfo_t)GetProcAddress(hModule, "clGetDeviceInfo");

    if (!(_clGetPlatformIDs && _clGetDeviceIDs && _clGetDeviceInfo)) return false;

    cl_uint numPlatforms = 0;
    _clGetPlatformIDs(0, nullptr, &numPlatforms);
    if (!numPlatforms) return false;

    std::vector<cl_platform_id> platforms(numPlatforms);
    _clGetPlatformIDs(numPlatforms, platforms.data(), nullptr);

    for (cl_uint i = 0; i < numPlatforms; i++) {
        cl_uint numDevices = 0;
        cl_int err = _clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, nullptr, &numDevices);

        if (err != CL_SUCCESS || !numDevices) continue;

        std::vector<cl_device_id> devices(numDevices);
        _clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, numDevices, devices.data(), nullptr);

        for (cl_uint j = 0; j < numDevices; ++j) {
            cl_device_id dev = devices[j];

            cl_bool luidValid = CL_FALSE;
            err = _clGetDeviceInfo(dev, CL_DEVICE_LUID_VALID, sizeof(cl_bool), &luidValid, nullptr);

            if (err != CL_SUCCESS) {
                _clGetDeviceInfo(dev, CL_DEVICE_LUID_VALID_KHR, sizeof(cl_bool), &luidValid, nullptr);
            }

            if (luidValid) {
                cl_uchar LUID[CL_LUID_SIZE] = { 0 };
                err = _clGetDeviceInfo(dev, CL_DEVICE_LUID, CL_LUID_SIZE, &LUID, nullptr);
                if (err != CL_SUCCESS) {
                    _clGetDeviceInfo(dev, CL_DEVICE_LUID_KHR, CL_LUID_SIZE, &LUID, nullptr);
                }

                if (!memcmp(&this->_adapterLUID, LUID, sizeof(LUID)))
                    return true;
            }
        }
    }

    return false;
}

bool GPU::_CheckOpenGL() {
    DISPLAY_DEVICEW ddw = { 0 };
    ddw.cb = sizeof(ddw);
    if (!EnumDisplayDevicesW(NULL, this->_adapterIndexForD3D9, &ddw, 0)) return false;

    if (!(ddw.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)) return false;

    DEVMODEW dm = { 0 };
    dm.dmSize = sizeof(dm);
    RECT rect = { 0, 0, 1, 1 };
    if (EnumDisplaySettingsW(ddw.DeviceName, ENUM_CURRENT_SETTINGS, &dm)) {
        rect.left = dm.dmPosition.x;
        rect.top = dm.dmPosition.y;
        rect.right = rect.left + 1;
        rect.bottom = rect.top + 1;
    }

    const wchar_t* clsName = L"DummyGLWindowClass";
    WNDCLASSEXW wc = { sizeof(wc) };
    wc.lpfnWndProc = DefWindowProcW;
    wc.lpszClassName = clsName;
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(0, clsName, L"", WS_POPUP, rect.left, rect.top, 1, 1, NULL, NULL, wc.hInstance, NULL);
    if (!hwnd) return false;

    bool hasOGL46 = false;
    HDC hDC = GetDC(hwnd);
    if (hDC) {
        PIXELFORMATDESCRIPTOR pfd = { sizeof(pfd), 1 };
        pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;

        int pf = ChoosePixelFormat(hDC, &pfd);
        if (pf && SetPixelFormat(hDC, pf, &pfd)) {
            HGLRC dummyContext = wglCreateContext(hDC);
            if (dummyContext && wglMakeCurrent(hDC, dummyContext)) {
                auto wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

                if (wglCreateContextAttribsARB) {
                    int attribs[] = {
                        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
                        WGL_CONTEXT_MINOR_VERSION_ARB, 6,
                        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                        0
                    };
                    HGLRC gl46 = wglCreateContextAttribsARB(hDC, NULL, attribs);
                    if (gl46) {
                        wglDeleteContext(gl46);
                        hasOGL46 = true;
                    }
                }
                wglMakeCurrent(NULL, NULL);
            }
            if (dummyContext) wglDeleteContext(dummyContext);
        }
        ReleaseDC(hwnd, hDC);
    }

    DestroyWindow(hwnd);
    return hasOGL46;
}

GPU::~GPU() {
	if (this->_pDXGIAdapter)
		this->_pDXGIAdapter->Release();

    if (this->_hDevInfo != INVALID_HANDLE_VALUE)
        SetupDiDestroyDeviceInfoList(this->_hDevInfo);
}