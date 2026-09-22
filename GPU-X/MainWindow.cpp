#include <dwmapi.h>
#include <windowsx.h>

#include "MainWindow.h"
#include "CommonID.h"
#include "Globals.h"
#include "VersionHelper.h"
#include "WindowHelpers.h"
#include "Themes.h"
#include "StringManager.h"

#include "resource.h"

ATOM MainWindow::_wndClass;
HWND MainWindow::_curComboBox;
MainWindow::MainWindowClassExtra MainWindow::_mwClsExtra;
std::vector<std::unique_ptr<GPU>> MainWindow::_vectorOfGPUs;

bool MainWindow::Init(std::vector<std::unique_ptr<GPU>>&& vectorOfGPUs) {
    MainWindow::_wndClass = CreateWindowClassW(L"GPUXWinClass", MainWindow::MainWindowProc, GetStockBrush(BLACK_BRUSH), IDC_ARROW, 0, 0);
    if (MainWindow::_wndClass) {
        MainWindow::_mwClsExtra.hBrush = CreateSolidBrush(currentTheme.standartGrayColor);
        MainWindow::_mwClsExtra.hCurrentThemeBrush = CreateSolidBrush(globalThemeColor);
        MainWindow::_mwClsExtra.hPen = CreatePen(PS_SOLID, 1, currentTheme.standartGrayColor);
        MainWindow::_mwClsExtra.hBorderPen = CreatePen(PS_SOLID, 1, globalThemeColor);
        MainWindow::_mwClsExtra.hNormalFont = CreateFontW(DPIManager::Scale(16), 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

        MainWindow::_vectorOfGPUs = std::move(vectorOfGPUs);
    }

    return MainWindow::_wndClass != 0;
}

void MainWindow::InitCardList(HWND hwnd) {
    for (const auto& gpu : MainWindow::_vectorOfGPUs) {
        ComboBox_AddString(hwnd, gpu->GetDeviceName().c_str());
    }

    MainWindow::_curComboBox = hwnd;
}

void MainWindow::DrawLabel(int x, int y, int w, const std::wstring& text, HDC WindowDC) {
    RECT r = { x, y, x + w, y + DPIManager::Scale(MainWindow::_mwClsExtra.rowHeight) };
    DrawTextW(WindowDC, text.c_str(), -1, &r, DT_SINGLELINE | DT_VCENTER | DT_RIGHT);
}

void MainWindow::DrawValueField(int x, int y, int w, const std::wstring& value, HDC WindowDC) {
    RECT r = { x, y, x + w, y + DPIManager::Scale(MainWindow::_mwClsExtra.rowHeight) };

    auto oldBrush = SelectObject(WindowDC, GetStockObject(NULL_BRUSH));
    Rectangle(WindowDC, r.left, r.top, r.right, r.bottom);
    SelectObject(WindowDC, oldBrush);

    DrawTextW(WindowDC, value.c_str(), -1, &r, DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_END_ELLIPSIS);
}

void MainWindow::DrawTechCheckbox(int x, int y, int w, const std::wstring& name, bool checked, HDC WindowDC) {
    RECT rBox = { x, y + DPIManager::Scale(4), x + DPIManager::Scale(14), y + DPIManager::Scale(18) };
    RECT rText = { x + DPIManager::Scale(20), y, x + w, y + DPIManager::Scale(MainWindow::_mwClsExtra.rowHeight) };

    auto oldBrush = SelectObject(WindowDC, GetStockObject(NULL_BRUSH));
    Rectangle(WindowDC, rBox.left, rBox.top, rBox.right, rBox.bottom);
    SelectObject(WindowDC, oldBrush);

    if (checked) {
        COLORREF oldColor = SetTextColor(WindowDC, currentTheme.standartGreenColor);
        DrawTextW(WindowDC, L"\u2713", -1, &rBox, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
        SetTextColor(WindowDC, oldColor);
    }

    DrawTextW(WindowDC, name.c_str(), -1, &rText, DT_SINGLELINE | DT_VCENTER | DT_LEFT);
}

void MainWindow::DrawGraphicsCard(HDC WindowDC) {
    const auto selIdx = ComboBox_GetCurSel(MainWindow::_curComboBox);
    if (selIdx < 0 || selIdx >= static_cast<int>(MainWindow::_vectorOfGPUs.size())) return;
    const auto& gpu = MainWindow::_vectorOfGPUs[selIdx];

    SetBkMode(WindowDC, TRANSPARENT);
    SetTextColor(WindowDC, currentTheme.standartWhiteColor);

    auto oldPen = SelectObject(WindowDC, MainWindow::_mwClsExtra.hBorderPen);
    auto oldFont = SelectObject(WindowDC, MainWindow::_mwClsExtra.hNormalFont);

    const int startX = DPIManager::Scale(MainWindow::_mwClsExtra.startX);
    const int labelW = DPIManager::Scale(MainWindow::_mwClsExtra.labelW);
    const int rowH = DPIManager::Scale(MainWindow::_mwClsExtra.rowHeight);
    const int rightPad = DPIManager::Scale(MainWindow::_mwClsExtra.rightPad);
    const int gap = DPIManager::Scale(MainWindow::_mwClsExtra.gap);
    const int correction = DPIManager::Scale(MainWindow::_mwClsExtra.correction);
    const int rightEdge = DPIManager::Scale(MainWindow::_mwClsExtra.rightEdge);

    int y = DPIManager::Scale(MainWindow::_mwClsExtra.startY);

    auto at = [&](int x0) -> MainWindow::Row { return MainWindow::Row{ WindowDC, x0, y, rightEdge, gap }; };

    at(startX).label(StringManager::GetStringByID(IDS_NAME), labelW).valueToEdge(gpu->GetDeviceName());
    y += rowH + gap;

    at(startX).label(StringManager::GetStringByID(IDS_REVISION), labelW)
        .value(gpu->GetRevision(), DPIManager::Scale(50))
        .label(StringManager::GetStringByID(IDS_DEVICEID), DPIManager::Scale(50))
        .value(gpu->GetDeviceID(), DPIManager::Scale(160));
    y += rowH + gap;

    at(startX- correction)
        .label(StringManager::GetStringByID(IDS_BIOS_VERSION), labelW+ correction)
        .valueToEdge(gpu->GetBIOSVersion());
    y += rowH + gap;

    at(startX).label(StringManager::GetStringByID(IDS_DRIVER_DATE), labelW)
        .value(gpu->GetDriverDate(), DPIManager::Scale(70), DPIManager::Scale(73) + gap)
        .label(StringManager::GetStringByID(IDS_DRIVER_VERSION), DPIManager::Scale(73))
        .value(gpu->GetDriverVersion(), DPIManager::Scale(115));
    y += rowH + gap;

    at(startX).label(StringManager::GetStringByID(IDS_BUS), labelW)
        .valueToEdge(gpu->GetBusCurrent() + L"/" + gpu->GetBusMaximum());
    y += rowH + gap;

    at(startX - correction).label(StringManager::GetStringByID(IDS_MEM_SIZE), labelW + correction)
        .value(std::to_wstring(gpu->GetMemSize()) + L" MB", DPIManager::Scale(77), DPIManager::Scale(73) + gap)
        .label(StringManager::GetStringByID(IDS_MEM_TYPE), DPIManager::Scale(73))
        .value(gpu->GetMemoryType(), DPIManager::Scale(115));
    y += rowH + 2*gap;

    at(startX- correction).label(StringManager::GetStringByID(IDS_GPU_CLOCK), labelW+ correction)
        .value(std::to_wstring(gpu->GetCoreClock()) + L" MHz", DPIManager::Scale(77), DPIManager::Scale(60))
        .label(StringManager::GetStringByID(IDS_MEM), DPIManager::Scale(50), DPIManager::Scale(53))
        .value(std::to_wstring(gpu->GetMemoryClock()) + L" MHz", DPIManager::Scale(65), DPIManager::Scale(35))
        .label(StringManager::GetStringByID(IDS_BOOST), DPIManager::Scale(65), DPIManager::Scale(68))
        .value(std::to_wstring(gpu->GetBoostCoreClock()) + L" MHz", DPIManager::Scale(65));
    y += rowH + gap;

    at(startX- correction).label(StringManager::GetStringByID(IDS_DEFAULT_CLOCK), labelW+ correction)
        .value(std::to_wstring(gpu->GetDefaultCoreClock()) + L" MHz", DPIManager::Scale(77), DPIManager::Scale(60))
        .label(StringManager::GetStringByID(IDS_MEM), DPIManager::Scale(50), DPIManager::Scale(53))
        .value(std::to_wstring(gpu->GetDefaultMemoryClock()) + L" MHz", DPIManager::Scale(65), DPIManager::Scale(35))
        .label(StringManager::GetStringByID(IDS_BOOST), DPIManager::Scale(65), DPIManager::Scale(68))
        .value(std::to_wstring(gpu->GetBoostMemoryClock()) + L" MHz", DPIManager::Scale(65));
    y += rowH + gap;

    at(startX - correction).label(StringManager::GetStringByID(IDS_RESIZABLE_BAR), labelW + correction)
        .value(gpu->GetHasResizableBAR() ? L"Enable" : L"Disable", DPIManager::Scale(77), DPIManager::Scale(95) + gap)
        .label(StringManager::GetStringByID(IDS_DX_SUPPORT), DPIManager::Scale(95))
        .value(gpu->GetDXMaxVersion(), DPIManager::Scale(70));
    y += rowH + 3*gap;

    at(startX - 4*correction).label(StringManager::GetStringByID(IDS_SUPPORT), labelW)
        .check(L"OpenCL", labelW, gpu->GetHasOpenCL(), labelW + correction)
        .check(L"CUDA", DPIManager::Scale(55), gpu->GetHasCUDA(), DPIManager::Scale(62))
        .check(L"PhysX", DPIManager::Scale(55), gpu->GetHasPhysX())
        .check(L"DirectCompute", DPIManager::Scale(110), gpu->GetHasDirectCompute());
    y += rowH + gap;

    at(startX - 4*correction + labelW + gap)
        .check(L"Vulkan", labelW, gpu->GetHasVulkan(), labelW)
        .check(L"OpenGL 4.6", DPIManager::Scale(85), gpu->GetHasVulkan(), DPIManager::Scale(85))
        .check(L"DirectML", DPIManager::Scale(67), gpu->GetHasDirectML(), DPIManager::Scale(74))
        .check(L"RayTracing", DPIManager::Scale(100), gpu->GetHasRayTracing());

    SelectObject(WindowDC, oldFont);
    SelectObject(WindowDC, oldPen);
}

void MainWindow::DrawSensors(HDC WindowDC) {
    const auto selIdx = ComboBox_GetCurSel(MainWindow::_curComboBox);
    if (selIdx < 0 || selIdx >= static_cast<int>(MainWindow::_vectorOfGPUs.size())) return;
    const auto& gpu = MainWindow::_vectorOfGPUs[selIdx];

    SetBkMode(WindowDC, TRANSPARENT);
    SetTextColor(WindowDC, currentTheme.standartWhiteColor);

    auto oldPen = SelectObject(WindowDC, MainWindow::_mwClsExtra.hBorderPen);
    auto oldFont = SelectObject(WindowDC, MainWindow::_mwClsExtra.hNormalFont);

    const int startX = DPIManager::Scale(MainWindow::_mwClsExtra.startX);
    const int labelW = DPIManager::Scale(MainWindow::_mwClsExtra.labelW);
    const int valueW = DPIManager::Scale(90);
    const int barW = DPIManager::Scale(180);
    const int mmW = DPIManager::Scale(140);
    const int rowH = DPIManager::Scale(MainWindow::_mwClsExtra.rowHeight);
    const int gap = DPIManager::Scale(MainWindow::_mwClsExtra.gap);
    const int rightEdge = DPIManager::Scale(MainWindow::_mwClsExtra.rightEdge);

    int y = DPIManager::Scale(MainWindow::_mwClsExtra.startY);
    auto at = [&](int x0) -> MainWindow::Row { return MainWindow::Row{ WindowDC, x0, y, rightEdge, gap }; };

    at(startX).label(StringManager::GetStringByID(IDS_GPU_TEMP), labelW).value(std::to_wstring(gpu->GetCoreTemperature()) + L" °C", labelW).graphic(barW, gpu->GetCoreTemperatureHistory(), 0u, 100u);
    y += rowH + gap;

    wchar_t buf[32];
    swprintf(buf, std::size(buf), L"%.2f V", gpu->GetCoreVoltage());
    at(startX - 5).label(StringManager::GetStringByID(IDS_GPU_VOLT), labelW + 5).value(buf, labelW).graphic(barW, gpu->GetCoreVoltageHistory(), 0.0, 2.0);
    y += rowH + gap;

    const auto& f = gpu->GetFanSpeed();
    for (int i = 0; i < f.size(); i++) {
        at(startX).label(StringManager::GetStringByID(IDS_FAN) + std::to_wstring(i + 1), labelW).value(std::to_wstring(f[i]) + L" RPM", labelW).graphic(barW, gpu->GetFanSpeedHistory()[i].data(), 0u, 4000u);
        y += rowH + gap;
    }

    SelectObject(WindowDC, oldFont);
    SelectObject(WindowDC, oldPen);
}

LRESULT WINAPI MainWindow::MainWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
    case WM_CREATE: {
        if (IsWindows10OrGreater()) {
            MARGINS margins = { 0 };
            margins.cyBottomHeight = IsWindows11OrGreater() ? DPIManager::Scale(AppSizeY) : 0;
            DwmExtendFrameIntoClientArea(hwnd, &margins);

            const auto DWMSBT_TABBEDWINDOW = 4;
            DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &DWMSBT_TABBEDWINDOW, sizeof(int));

            const auto DARK = 1;
            DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &DARK, sizeof(int));

        }

        SetTimer(hwnd, 0, 1000, 0);
        break;
    }
    case WM_GETMINMAXINFO: {
        MINMAXINFO* pInfo = (MINMAXINFO*)lparam;
        pInfo->ptMinTrackSize.x = DPIManager::Scale(AppSizeX);
        pInfo->ptMaxTrackSize.x = DPIManager::Scale(AppSizeX);
        pInfo->ptMinTrackSize.y = DPIManager::Scale(AppSizeY);
        pInfo->ptMaxTrackSize.y = DPIManager::Scale(AppSizeY);
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

                if (pt.y < DPIManager::Scale(30)) return HTCAPTION;
                else return HTCLIENT;
            }
            return lRet;
        }
        else return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
    case WM_ERASEBKGND: return 1;
    case WM_PAINT: {
        PAINTSTRUCT PS;
        HDC WindowDC = BeginPaint(hwnd, &PS);

        FillRect(WindowDC, &PS.rcPaint, GetStockBrush(BLACK_BRUSH));

        auto oldBrush = SelectObject(WindowDC, MainWindow::_mwClsExtra.hBrush);
        auto oldPen = SelectObject(WindowDC, MainWindow::_mwClsExtra.hPen);

        Rectangle(WindowDC, DPIManager::Scale(5), DPIManager::Scale(TabHeight + 15), DPIManager::Scale(AppSizeX - 5), 
            DPIManager::Scale(AppSizeY - ComboBoxHeight - 15));

        switch (MainWindow::_mwClsExtra.currentTab) {
        case 0: {
            MainWindow::DrawGraphicsCard(WindowDC);
            break;
        }
        case 1: {
            MainWindow::DrawSensors(WindowDC);
            break;
        }
        }

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
        if ((HMENU)wparam == CLOSE_BUTTON) DestroyWindow(hwnd);
        else if (HIWORD(wparam) == CBN_SELCHANGE) {
            InvalidateRect(hwnd, 0, 1);
        }
        else {
            MainWindow::_mwClsExtra.currentTab = wparam;
            InvalidateRect(hwnd, 0, 1);
        }
        break;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        break;
    }
    case WM_TIMER: {
        const auto selIdx = ComboBox_GetCurSel(MainWindow::_curComboBox);
        if (selIdx < 0 || selIdx >= static_cast<int>(MainWindow::_vectorOfGPUs.size())) break;
        MainWindow::_vectorOfGPUs[selIdx]->UpdateSensors();
        InvalidateRect(hwnd, 0, 1);
        break;
    }
    default: return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
    return 0;
}

void MainWindow::Release() {
    if (MainWindow::_wndClass) {
        DeleteObject(MainWindow::_mwClsExtra.hNormalFont);
        DeleteObject(MainWindow::_mwClsExtra.hBorderPen);
        DeleteObject(MainWindow::_mwClsExtra.hPen);
        DeleteObject(MainWindow::_mwClsExtra.hCurrentThemeBrush);
        DeleteObject(MainWindow::_mwClsExtra.hBrush);
        UnregisterClassW(L"GPUXWinClass", 0);
    }
}