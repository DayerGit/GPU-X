#include <iostream>

#include <Windows.h>
#include <Windowsx.h>
#include <dwmapi.h>

#include "VersionHelper.h"
#include "GPUFactory.h"

#define AppSizeX 400
#define AppSizeY 500

#define TabHeight 40
#define TabRound 15

#define GC_TAB HMENU(0)
#define S_TAB HMENU(1)

extern "C" {
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
}

int globalStateCurrentTab = 0;

LRESULT WINAPI MainWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    static HBRUSH brush;
    static HPEN Pen;

    switch (msg) {
    case WM_CREATE: {
        if (IsWindows10OrGreater()) {
            MARGINS margins = { 0 };
            margins.cyBottomHeight = IsWindows11OrGreater() ? AppSizeY : 0;
            DwmExtendFrameIntoClientArea(hwnd, &margins);

            const auto DWMSBT_TABBEDWINDOW = 4;
            DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &DWMSBT_TABBEDWINDOW, sizeof(int));

            const auto DARK = 1;
            DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &DARK, sizeof(int));
        }

        brush = CreateSolidBrush(RGB(45, 45, 45));
        Pen = CreatePen(PS_SOLID, 1, RGB(45, 45, 45));
        break;
    }
    case WM_GETMINMAXINFO: {
        MINMAXINFO* pInfo = (MINMAXINFO*)lparam;
        pInfo->ptMinTrackSize.x = AppSizeX;
        pInfo->ptMaxTrackSize.x = AppSizeX;
        pInfo->ptMinTrackSize.y = AppSizeY;
        pInfo->ptMaxTrackSize.y = AppSizeY;
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT PS;
        HDC WindowDC = BeginPaint(hwnd, &PS);
        
        auto oldBrush = SelectObject(WindowDC, brush);
        auto oldPen = SelectObject(WindowDC, Pen);
        
        Rectangle(WindowDC, 5, TabHeight, AppSizeX - 20, AppSizeY - TabHeight);

        SelectObject(WindowDC, oldBrush);
        SelectObject(WindowDC, oldPen);

        EndPaint(hwnd, &PS);
        break;
    }
    case WM_COMMAND: {
        globalStateCurrentTab = wparam;
        InvalidateRect(hwnd, 0, 1);
        break;
    }
    case WM_DESTROY: {
        DeleteObject(brush);
        DeleteObject(Pen);
        PostQuitMessage(0);
        break;
    }
    default: return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
	return 0;
}

LRESULT WINAPI MyButtonWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    static HBRUSH Brushes[3];
    static int current = 1;
    static bool isMouseInside = false;
    static HFONT hFont = NULL;

    switch (msg) {
    case WM_CREATE: {
        Brushes[0] = CreateSolidBrush(RGB(45, 45, 45));
        Brushes[1] = CreateSolidBrush(RGB(0, 0, 0));
        Brushes[2] = CreateSolidBrush(RGB(10, 10, 10));
        
        int len = GetWindowTextLengthW(hwnd) + 1;
        auto WindowName = new wchar_t[len];

        GetWindowTextW(hwnd, WindowName, len);

        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)WindowName);
        break;
    }
    case WM_PAINT: {
        auto WindowName = (wchar_t*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
        
        PAINTSTRUCT PS;
        HDC WindowDC = BeginPaint(hwnd, &PS);
        
        FillRect(WindowDC, &PS.rcPaint, Brushes[globalStateCurrentTab == (int)GetMenu(hwnd) ? 0 : current]);

        auto fontToUse = hFont ? hFont : GetStockObject(DEFAULT_GUI_FONT);
        auto oldFont = SelectObject(WindowDC, fontToUse);

        SetTextColor(WindowDC, RGB(255, 255, 255)); 
        SetBkMode(WindowDC, TRANSPARENT);
        DrawTextW(WindowDC, WindowName, -1, &PS.rcPaint, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
        
        SelectObject(WindowDC, oldFont);
        EndPaint(hwnd, &PS);
        break;
    }
    case WM_MOUSEMOVE: {
        if (!isMouseInside) {
            isMouseInside = true;

            TRACKMOUSEEVENT leaveTME = { 0 };
            leaveTME.cbSize = sizeof(leaveTME);
            leaveTME.dwFlags = TME_LEAVE;
            leaveTME.hwndTrack = hwnd;
            TrackMouseEvent(&leaveTME);

            current = 2;
            InvalidateRect(hwnd, 0, 1);
        }
        break;
    }
    case WM_MOUSELEAVE: {
        current = 1;
        isMouseInside = false;

        InvalidateRect(hwnd, 0, 1);
        break;
    }
    case WM_LBUTTONDOWN: {
        current = 1;
        
        SendMessageW(GetParent(hwnd), WM_COMMAND, (WPARAM)GetMenu(hwnd), 0);
        break;
    }
    case WM_SETFONT: {
        hFont = (HFONT)wparam;
        if (lparam) InvalidateRect(hwnd, 0, 1);
        break;
    }
    case WM_DESTROY: {
        for (int i = 0; i < ARRAYSIZE(Brushes); i++) {
            DeleteObject(Brushes[i]);
        }
        delete[] (wchar_t*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
        break;
    }
    default: return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
    return 0;
}

ATOM CreateWindowClassW(LPCWSTR Name, WNDPROC ClassProc, HBRUSH BackgroundBrush, LPCWSTR Cursor, LPWSTR Icon, int extra) {
    WNDCLASSW wcl;
    memset(&wcl, 0, sizeof(wcl));

    wcl.lpszClassName = Name;
    wcl.lpfnWndProc = ClassProc;
    wcl.hbrBackground = BackgroundBrush;
    wcl.hCursor = LoadCursorW(0, Cursor);
    wcl.cbWndExtra = extra;
    wcl.hIcon = LoadIconW(GetModuleHandle(0), MAKEINTRESOURCEW(Icon));

    return RegisterClassW(&wcl);
}

int main() {

    /*if(!CreateWindowClassW(L"GPUXWinClass", MainWindowProc, GetStockBrush(BLACK_BRUSH), IDC_ARROW, 0, 
        sizeof(std::vector<std::unique_ptr<GPU>>))) return -1;

    if(!CreateWindowClassW(L"GPUXButtonClass", MyButtonWindowProc, GetStockBrush(BLACK_BRUSH), IDC_ARROW, 0, sizeof(wchar_t*))) return -1;

    int screenSizeX = GetSystemMetrics(SM_CXFULLSCREEN);
    int screenSizeY = GetSystemMetrics(SM_CYFULLSCREEN);


    HWND Window = CreateWindowExW(WS_EX_DLGMODALFRAME, L"GPUXWinClass", L"GPU-X", WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, (screenSizeX - AppSizeX) / 2, (screenSizeY - AppSizeY) / 2, AppSizeX, AppSizeY, 0, 0, 0, 0);
    
    HWND GraphicsCardTabButton = CreateWindowExW(0, L"GPUXButtonClass", L"Graphics Card", WS_CHILD | WS_VISIBLE, 5, 0, 100, TabHeight, Window, GC_TAB, 0, 0);
    HRGN GraphicsCardTabRegion = CreateRoundRectRgn(0, 0, 100, TabHeight + 10, TabRound, TabRound);
    SetWindowRgn(GraphicsCardTabButton, GraphicsCardTabRegion, TRUE);

    HWND SensorsTabButton = CreateWindowExW(0, L"GPUXButtonClass", L"Sensors", WS_CHILD | WS_VISIBLE, 110, 0, 80, TabHeight, Window, S_TAB, 0, 0);
    HRGN SensorsTabRegion = CreateRoundRectRgn(0, 0, 80, TabHeight + 10, TabRound, TabRound);
    SetWindowRgn(SensorsTabButton, SensorsTabRegion, TRUE);

    HFONT hNormalFont = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    SetWindowFont(GraphicsCardTabButton, hNormalFont, 1);
    SetWindowFont(SensorsTabButton, hNormalFont, 1);*/

    GPUFactory factory;
    auto res = factory.LetsCreateGPUs();

    for (const auto& i : res) {
        std::wcout << std::setw(5) << std::setfill(L'=') << " " << i->GetDeviceName() << " " << std::setw(5) << std::setfill(L'=') << " " << std::endl;
        std::wcout << "  * DeviceID: " << i->GetDeviceID() << std::endl;
        std::wcout << "  * Revision: " << i->GetRevision() << std::endl;
        std::wcout << "  * DriverVersion: " << i->GetDriverVersion() << std::endl;
        std::wcout << "  * DriverDate: " << i->GetDriverDate() << std::endl;
        std::wcout << "  * MemSize: " << i->GetMemSize() << " MB" << std::endl;
        std::wcout << "  * D3D Version: " << i->GetDXMaxVersion() << std::endl;
        std::wcout << "  * RayTracing: " << i->GetHasRayTracing() << std::endl;
        std::wcout << "  * DirectCompute: " << i->GetHasDirectCompute() << std::endl;
        std::wcout << "  * DirectML: " << i->GetHasDirectML() << std::endl;
        std::wcout << "  * VULKAN: " << i->GetHasVulkan() << std::endl;
        std::wcout << "  * OpenCL: " << i->GetHasOpenCL() << std::endl;
        std::wcout << "  * OpenGL 4.6: " << i->GetHasOGL4_6() << std::endl;
        std::wcout << "  * Resizable BAR: " << i->GetHasResizableBAR() << std::endl;

        std::wcout << "  * BIOS Version: " << i->GetBIOSVersion() << std::endl;
        std::wcout << "  * Memory Type: " << i->GetMemoryType() << std::endl;
        std::wcout << "  * Core Clock: " << i->GetCoreClock() << std::endl;
        std::wcout << "  * Core Default Clock: " << i->GetDefaultCoreClock() << std::endl;
        std::wcout << "  * Core Boost Clock: " << i->GetBoostCoreClock() << std::endl;
        std::wcout << "  * Memory Clock: " << i->GetMemoryClock() << std::endl;
        std::wcout << "  * Memory Default Clock: " << i->GetDefaultMemoryClock() << std::endl;
        std::wcout << "  * Memory Boost Clock: " << i->GetBoostMemoryClock() << std::endl;
        std::wcout << "  * Fan Speed: " << std::endl;
        std::wcout << "  * Bus Maximum: " << i->GetBusMaximum() << std::endl;
        std::wcout << "  * Bus Current: " << i->GetBusCurrent() << std::endl;
        std::wcout << "  * Core Temp: " << i->GetCoreTemperature() << std::endl;
        std::wcout << "  * Core Voltage: " << i->GetCoreVoltage() << " V" << std::endl;
        std::wcout << "  * CUDA: " << i->GetHasCUDA() << std::endl;
        std::wcout << "  * PhysX: " << i->GetHasPhysX() << std::endl;
        const auto& fanSpeed = i->GetFanSpeed();
        for (int j = 0; j < fanSpeed.size(); j++) {
            std::wcout << "    - Fan #" << j + 1 << ": " << fanSpeed[j] << std::endl;
        }
        
        /*switch (i->GetProducer()) {
        case TypeOfGPU::INTEL_GPU: {
            Intel_GPU* intelGPU = static_cast<Intel_GPU*>(i.get());
            std::wcout << "  * BIOS Version: " << intelGPU->GetBIOSVersion() << std::endl;
            std::wcout << "  * Memory Type: " << intelGPU->GetMemoryType() << std::endl;
            std::wcout << "  * GPU Clock: " << intelGPU->GetCoreClock() << std::endl;
            std::wcout << "  * GPU Default Clock: " << intelGPU->GetDefaultCoreClock() << std::endl;
            std::wcout << "  * GPU Boost Clock: " << intelGPU->GetBoostCoreClock() << std::endl;
            std::wcout << "  * Memory Clock: " << intelGPU->GetMemoryClock() << std::endl;
            std::wcout << "  * Memory Default Clock: " << intelGPU->GetDefaultMemoryClock() << std::endl;
            std::wcout << "  * Memory Boost Clock: " << intelGPU->GetBoostMemoryClock() << std::endl;
            const auto& fanSpeed = intelGPU->GetFanSpeed();
            for (int i = 0; i < fanSpeed.size(); i++) {
                std::wcout << "    - Fan #" << i + 1 << ": " << fanSpeed[i] << std::endl;
            }
            std::wcout << "  * Bus Maximum: " << intelGPU->GetBusMaximum() << std::endl;
            std::wcout << "  * Bus Current: " << intelGPU->GetBusCurrent() << std::endl;
            break;
        }
        case TypeOfGPU::NVIDIA_GPU: {
            NVIDIA_GPU* NvidiaGPU = static_cast<NVIDIA_GPU*>(i.get());
            std::wcout << "  * BIOS Version: " << NvidiaGPU->GetBIOSVersion() << std::endl;
            std::wcout << "  * Memory Type: " << NvidiaGPU->GetMemoryType() << std::endl;
            std::wcout << "  * Core Clock: " << NvidiaGPU->GetCoreClock() << std::endl;
            std::wcout << "  * Core Default Clock: " << NvidiaGPU->GetDefaultCoreClock() << std::endl;
            std::wcout << "  * Core Boost Clock: " << NvidiaGPU->GetBoostCoreClock() << std::endl;
            std::wcout << "  * Memory Clock: " << NvidiaGPU->GetMemoryClock() << std::endl;
            std::wcout << "  * Memory Default Clock: " << NvidiaGPU->GetDefaultMemoryClock() << std::endl;
            std::wcout << "  * Memory Boost Clock: " << NvidiaGPU->GetBoostMemoryClock() << std::endl;
            std::wcout << "  * Fan Speed: " << std::endl;
            const auto& fanSpeed = NvidiaGPU->GetFanSpeed();
            for (int i = 0; i < fanSpeed.size(); i++) {
                std::wcout << "    - Fan #" << i + 1 << ": " << fanSpeed[i] << std::endl;
            }
            std::wcout << "  * Bus Maximum: " << NvidiaGPU->GetBusMaximum() << std::endl;
            std::wcout << "  * Bus Current: " << NvidiaGPU->GetBusCurrent() << std::endl;
            std::wcout << "  * Core Temp: " << NvidiaGPU->GetCoreTemperature() << std::endl;
            std::wcout << "  * Core Voltage: " << NvidiaGPU->GetCoreVoltage() << " V" << std::endl;
            std::wcout << "  * CUDA: " << NvidiaGPU->GetHasCUDA() << std::endl;
            std::wcout << "  * PhysX: " << NvidiaGPU->GetHasPhysX() << std::endl;
            break;
        }
        case TypeOfGPU::AMD_GPU: {
            AMD_GPU* AmdGPU = static_cast<AMD_GPU*>(i.get());
            std::wcout << "  * BIOS Version: " << AmdGPU->GetBIOSVersion() << std::endl;
            break;
        }
        }*/

        std::cout << std::endl;
    }

    /*ShowWindow(Window, SW_SHOWNORMAL);

    MSG msg;
    while (GetMessageW(&msg, 0, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }*/
    return 0;
}