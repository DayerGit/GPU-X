#include <iostream>

#include <Windows.h>
#include <Windowsx.h>
#include <dwmapi.h>

#include "VersionHelper.h"
#include "GPUFactory.h"

#include "resource.h"

#define AppSizeX 400
#define AppSizeY 500

#define TabHeight 40
#define TabRound 15

#define ComboBoxHeight 35
#define indentComboBox 15
#define ComboBoxRound 10

#define ComboBoxStateMinimize 0
#define ComboBoxStateMaximize 1

#define GC_TAB HMENU(0)
#define S_TAB HMENU(1)
#define CLOSE_BUTTON HMENU(2)
#define CL_COMBOBOX HMENU(3)

extern "C" {
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
}

int globalStateCurrentTab = 0;

COLORREF globalThemeColor = RGB(0, 0, 0);

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
        DWORD rawDwmColor = 0;
        BOOL paid;

        if (SUCCEEDED(DwmGetColorizationColor(&rawDwmColor, &paid))) {
            BYTE r = (rawDwmColor >> 16) & 0xFF;
            BYTE g = (rawDwmColor >> 8) & 0xFF;
            BYTE b = rawDwmColor & 0xFF;

            globalThemeColor = RGB(r, g, b);
        }
        else globalThemeColor = RGB(0, 120, 215);

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
    case WM_NCCALCSIZE: {
        return IsWindows10OrGreater() ? 0 : DefWindowProcW(hwnd, msg, wparam, lparam);
    }
    case WM_NCHITTEST: {
        if (IsWindows10OrGreater()) {
            LRESULT lRet = 0;
            DwmDefWindowProc(hwnd, msg, wparam, lparam, &lRet);

            if (0 == lRet) {
                POINT pt = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
                ScreenToClient(hwnd, &pt);

                if (pt.y < 30) return HTCAPTION;
                else return HTCLIENT;
            }
            return lRet;
        }
        else return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
    case WM_PAINT: {
        PAINTSTRUCT PS;
        HDC WindowDC = BeginPaint(hwnd, &PS);

        FillRect(WindowDC, &PS.rcPaint, GetStockBrush(BLACK_BRUSH));
        
        auto oldBrush = SelectObject(WindowDC, brush);
        auto oldPen = SelectObject(WindowDC, Pen);
        
        Rectangle(WindowDC, 5, TabHeight + 15, AppSizeX - 5, AppSizeY - ComboBoxHeight - 15);

        SelectObject(WindowDC, oldBrush);
        SelectObject(WindowDC, oldPen);

        EndPaint(hwnd, &PS);
        break;
    }
    case WM_LBUTTONDOWN: {
        SetFocus(hwnd); 
        break;
    }
    case WM_COMMAND: {
        if ((HMENU)wparam == CLOSE_BUTTON) PostQuitMessage(0);
        else {
            globalStateCurrentTab = wparam;
            InvalidateRect(hwnd, 0, 1);
        }
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


struct CustomButtonInfo {
    wchar_t* windowName;
    HFONT hFont;
};

LRESULT WINAPI MyButtonWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    static HBRUSH Brushes[3];
    static int current = 1;
    static bool isMouseInside = false;

    auto cbi = (CustomButtonInfo*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    switch (msg) {
    case WM_CREATE: {
        Brushes[0] = CreateSolidBrush(RGB(45, 45, 45));
        Brushes[1] = CreateSolidBrush(RGB(0, 0, 0));
        Brushes[2] = CreateSolidBrush(RGB(10, 10, 10));

        CustomButtonInfo* cbi = new CustomButtonInfo;
        
        int len = GetWindowTextLengthW(hwnd) + 1;
        auto WindowName = new wchar_t[len];

        GetWindowTextW(hwnd, WindowName, len);

        cbi->windowName = WindowName;
        cbi->hFont = GetStockFont(DEFAULT_GUI_FONT);

        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)cbi);
        break;
    }
    case WM_PAINT: {
         
        PAINTSTRUCT PS;
        HDC WindowDC = BeginPaint(hwnd, &PS);
        
        FillRect(WindowDC, &PS.rcPaint, Brushes[globalStateCurrentTab == (int)GetMenu(hwnd) ? 0 : current]);

        auto fontToUse = cbi ? cbi->hFont : GetStockObject(DEFAULT_GUI_FONT);
        auto oldFont = SelectObject(WindowDC, fontToUse);

        RECT rcClient;
        GetClientRect(hwnd, &rcClient);

        SetTextColor(WindowDC, RGB(255, 255, 255)); 
        SetBkMode(WindowDC, TRANSPARENT);
        if(cbi)
            DrawTextW(WindowDC, cbi->windowName, -1, &rcClient, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
        
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
        SetFocus(hwnd);
        break;
    }
    case WM_SETFONT: {
        
        if(cbi) cbi->hFont = (HFONT)wparam;
        if (lparam) InvalidateRect(hwnd, 0, 1);
        break;
    }
    case WM_DESTROY: {
        for (int i = 0; i < ARRAYSIZE(Brushes); i++) {
            DeleteObject(Brushes[i]);
        }

        if (cbi) {
            delete[] cbi->windowName;
            delete cbi;
        }
        break;
    }
    default: return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
    return 0;
}


struct CustomComboBoxInfo {
    int currentIndex, currentState;
    std::vector<std::wstring> strings;
    HRGN ComboBoxRegion;
    HFONT hFont;
};

LRESULT WINAPI MyComboBoxWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    static HBRUSH brush = GetStockBrush(BLACK_BRUSH), themeColorBrush = GetStockBrush(BLACK_BRUSH);
    static HPEN pen = GetStockPen(BLACK_PEN);
    static HFONT font = GetStockFont(DEFAULT_GUI_FONT);

    auto ccbi = (CustomComboBoxInfo*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    switch (msg) {
    case WM_CREATE: {
        brush = CreateSolidBrush(RGB(80, 80, 80));
        themeColorBrush = CreateSolidBrush(globalThemeColor);
        pen = CreatePen(PS_SOLID, 1, RGB(80, 80, 80));
        font = CreateFontW(8, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe MDL2 Assets");

        CustomComboBoxInfo* ccbi = new CustomComboBoxInfo;
        ccbi->currentIndex = 0;
        ccbi->currentState = ComboBoxStateMinimize;

        RECT wndClient = { 0 };
        GetClientRect(hwnd, &wndClient);

        ccbi->ComboBoxRegion = CreateRoundRectRgn(0, 0, wndClient.right, ComboBoxHeight, ComboBoxRound, ComboBoxRound);
        SetWindowRgn(hwnd, ccbi->ComboBoxRegion, TRUE);

        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)ccbi);
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT PS;
        HDC WindowDC = BeginPaint(hwnd, &PS);

        RECT rcClient;
        GetClientRect(hwnd, &rcClient);
        int windowWidth = rcClient.right - rcClient.left;
        int windowHeight = rcClient.bottom - rcClient.top;

        FillRect(WindowDC, &PS.rcPaint, GetStockBrush(BLACK_BRUSH));

        auto oldBrush = SelectObject(WindowDC, brush);
        auto oldPen = SelectObject(WindowDC, pen);

        Rectangle(WindowDC, 0, 0, windowWidth, windowHeight);

        SetTextColor(WindowDC, RGB(255, 255, 255));
        SetBkMode(WindowDC, TRANSPARENT);

        if (ccbi && !ccbi->strings.empty()) {
            SelectObject(WindowDC, ccbi->hFont);

            if (ComboBoxStateMinimize == ccbi->currentState) {
                RECT rcText = rcClient;
                rcText.left += indentComboBox;
                rcText.right -= indentComboBox;

                DrawTextW(WindowDC, ccbi->strings[ccbi->currentIndex].c_str(), -1, &rcText, DT_SINGLELINE | DT_LEFT | DT_VCENTER);

                auto oldFont = SelectObject(WindowDC, font);
                DrawTextW(WindowDC, L"\uE70D\0", -1, &rcText, DT_SINGLELINE | DT_RIGHT | DT_VCENTER);
                SelectObject(WindowDC, oldFont);
            }
            else {
                int currentY = rcClient.bottom;

                for (int i = ccbi->strings.size() - 1; i >= 0; i--) {
                    RECT rcItem;
                    rcItem.left = rcClient.left + indentComboBox;
                    rcItem.right = rcClient.right - indentComboBox;

                    rcItem.bottom = currentY;
                    rcItem.top = currentY - ComboBoxHeight;

                    DrawTextW(WindowDC, ccbi->strings[i].c_str(), -1, &rcItem, DT_SINGLELINE | DT_LEFT | DT_VCENTER);

                    if (i == ccbi->currentIndex) {
                        auto oldBr = SelectObject(WindowDC, themeColorBrush);

                        Rectangle(WindowDC, 5, i * ComboBoxHeight + 10, 10, i * ComboBoxHeight + 25);

                        SelectObject(WindowDC, oldBr);
                    }

                    currentY -= ComboBoxHeight;
                }
            }
        }

        SelectObject(WindowDC, oldPen);
        SelectObject(WindowDC, oldBrush);

        EndPaint(hwnd, &PS);
        break;
    }
    case WM_LBUTTONDOWN: {
        if (ccbi && !ccbi->strings.empty()) {
            int X = 0, Y = 0, cx = 0, cy = 0;

            SetFocus(hwnd);

            RECT parentRect = { 0 };
            RECT thisRect = { 0 };

            HWND hParent = GetParent(hwnd);
            GetClientRect(hParent, &parentRect);
            GetWindowRect(hwnd, &thisRect);
            cx = thisRect.right - thisRect.left;

            if (ComboBoxStateMinimize == ccbi->currentState) {
                int currentHeight = thisRect.bottom - thisRect.top;

                POINT topLeft = { thisRect.left, thisRect.top };
                ScreenToClient(hParent, &topLeft);

                int dropDownHeight = ComboBoxHeight * (ccbi->strings.size() - 1);
                X = topLeft.x;
                Y = topLeft.y - dropDownHeight;
                cy = dropDownHeight + currentHeight;

                ccbi->currentState = ComboBoxStateMaximize;
            }
            else {
                X = thisRect.left;

                POINT topLeft = { thisRect.left, thisRect.top };
                ScreenToClient(hParent, &topLeft);

                POINT downRight = { thisRect.right, thisRect.bottom };
                ScreenToClient(hParent, &downRight);

                X = topLeft.x;

                Y = downRight.y - ComboBoxHeight;
                cy = ComboBoxHeight; 

                int clickY = GET_Y_LPARAM(lparam);

                ccbi->currentIndex = clickY / ComboBoxHeight;
                ccbi->currentState = ComboBoxStateMinimize;
            }
            SetWindowPos(hwnd, NULL, X, Y, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);

            DeleteObject(ccbi->ComboBoxRegion);
            ccbi->ComboBoxRegion = CreateRoundRectRgn(0, 0, cx, cy, ComboBoxRound, ComboBoxRound);
            SetWindowRgn(hwnd, ccbi->ComboBoxRegion, TRUE);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        break;
    }
    case WM_KILLFOCUS: {
        if (ccbi && ComboBoxStateMaximize == ccbi->currentState) {
            RECT thisRect = { 0 };
            HWND hParent = GetParent(hwnd);
            GetWindowRect(hwnd, &thisRect);

            int cx = thisRect.right - thisRect.left;

            POINT downRight = { thisRect.right, thisRect.bottom };
            ScreenToClient(hParent, &downRight);

            POINT topLeft = { thisRect.left, thisRect.top };
            ScreenToClient(hParent, &topLeft);

            int X = topLeft.x;
            int Y = downRight.y - ComboBoxHeight;
            int cy = ComboBoxHeight;

            ccbi->currentState = ComboBoxStateMinimize;

            SetWindowPos(hwnd, NULL, X, Y, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);

            DeleteObject(ccbi->ComboBoxRegion);
            ccbi->ComboBoxRegion = CreateRoundRectRgn(0, 0, cx, cy, ComboBoxRound, ComboBoxRound);
            SetWindowRgn(hwnd, ccbi->ComboBoxRegion, TRUE);

            InvalidateRect(hwnd, NULL, TRUE);
        }
        break;
    }
    case WM_SETFONT: {
        if (ccbi) ccbi->hFont = (HFONT)wparam;
        if (lparam) InvalidateRect(hwnd, 0, 1);
        break;
    }
    case CB_ADDSTRING: {
        if (ccbi)
            ccbi->strings.emplace_back((wchar_t*)lparam);
        break;
    }
    case WM_DESTROY: {
        DeleteObject(font);
        DeleteObject(pen);
        DeleteObject(themeColorBrush);
        DeleteObject(brush);

        if (ccbi) 
            delete ccbi;

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

    SetProcessDPIAware();

    if(!CreateWindowClassW(L"GPUXWinClass", MainWindowProc, GetStockBrush(BLACK_BRUSH), IDC_ARROW, 0, 0)) return -1;
    if(!CreateWindowClassW(L"GPUXButtonClass", MyButtonWindowProc, GetStockBrush(BLACK_BRUSH), IDC_ARROW, 0, sizeof(CustomButtonInfo*))) return -1;
    if(!CreateWindowClassW(L"GPUXComboBox", MyComboBoxWindowProc, GetStockBrush(BLACK_BRUSH), IDC_ARROW, 0, sizeof(CustomComboBoxInfo*))) return -1;

    int screenSizeX = GetSystemMetrics(SM_CXFULLSCREEN);
    int screenSizeY = GetSystemMetrics(SM_CYFULLSCREEN);


    HWND Window = CreateWindowExW(WS_EX_DLGMODALFRAME, L"GPUXWinClass", L"GPU-X", WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, (screenSizeX - AppSizeX) / 2, (screenSizeY - AppSizeY) / 2, AppSizeX, AppSizeY, 0, 0, 0, 0);
    
    wchar_t graphicsCardLabel[256], sensorsLabel[256];
    LoadStringW(0, IDS_GRAPHICS_CARD, graphicsCardLabel, 256);
    LoadStringW(0, IDS_SENSORS, sensorsLabel, 256);

    HWND GraphicsCardTabButton = CreateWindowExW(0, L"GPUXButtonClass", graphicsCardLabel, WS_CHILD | WS_VISIBLE, 5, 15, 120, TabHeight, Window, GC_TAB, 0, 0);
    HRGN GraphicsCardTabRegion = CreateRoundRectRgn(0, 0, 120, TabHeight + 10, TabRound, TabRound);
    SetWindowRgn(GraphicsCardTabButton, GraphicsCardTabRegion, TRUE);

    HWND SensorsTabButton = CreateWindowExW(0, L"GPUXButtonClass", sensorsLabel, WS_CHILD | WS_VISIBLE, 130, 15, 80, TabHeight, Window, S_TAB, 0, 0);
    HRGN SensorsTabRegion = CreateRoundRectRgn(0, 0, 80, TabHeight + 10, TabRound, TabRound);
    SetWindowRgn(SensorsTabButton, SensorsTabRegion, TRUE);

    HWND CardsListComboBox = CreateWindowExW(0, L"GPUXComboBox", L"CardList", WS_CHILD | WS_VISIBLE, 5, AppSizeY - ComboBoxHeight - 10, AppSizeX - 10, ComboBoxHeight, Window, CL_COMBOBOX, 0, 0);

    HFONT hBoldFont = CreateFontW(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT hNormalFont = CreateFontW(20, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT hControlFont = NULL;

    SetWindowFont(GraphicsCardTabButton, hBoldFont, 1);
    SetWindowFont(SensorsTabButton, hBoldFont, 1);
    SetWindowFont(CardsListComboBox, hNormalFont, 1);
    
    if (IsWindows10OrGreater()) {
        HWND CloseButton = CreateWindowExW(0, L"GPUXButtonClass", L"\uE106", WS_CHILD | WS_VISIBLE, AppSizeX - 45, 0, 45, 32, Window, CLOSE_BUTTON, 0, 0);
        hControlFont = CreateFontW(10, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe MDL2 Assets");

        SetWindowFont(CloseButton, hControlFont, 1);
    }

    GPUFactory factory;
    auto res = factory.LetsCreateGPUs();

    for (const auto& i : res) {
        ComboBox_AddString(CardsListComboBox, i->GetDeviceName().c_str());
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
        std::wcout << "  * Bus Maximum: " << i->GetBusMaximum() << std::endl;
        std::wcout << "  * Bus Current: " << i->GetBusCurrent() << std::endl;
        std::wcout << "  * Core Temp: " << i->GetCoreTemperature() << std::endl;
        std::wcout << "  * Core Voltage: " << i->GetCoreVoltage() << " V" << std::endl;
        std::wcout << "  * CUDA: " << i->GetHasCUDA() << std::endl;
        std::wcout << "  * PhysX: " << i->GetHasPhysX() << std::endl;
        std::wcout << "  * Fan Speed: " << std::endl;
        const auto& fanSpeed = i->GetFanSpeed();
        for (int j = 0; j < fanSpeed.size(); j++) {
            std::wcout << "    - Fan #" << j + 1 << ": " << fanSpeed[j] << std::endl;
        }

        std::cout << std::endl;
    }

    ShowWindow(Window, SW_SHOWNORMAL);

    MSG msg;
    while (GetMessageW(&msg, 0, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    DeleteObject(hNormalFont);
    if(hControlFont)
        DeleteObject(hControlFont);

    return 0;
}