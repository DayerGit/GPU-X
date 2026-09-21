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
        MainWindow::_mwClsExtra.hPen = CreatePen(PS_SOLID, 1, currentTheme.standartGrayColor);
        MainWindow::_mwClsExtra.hBorderPen = CreatePen(PS_SOLID, 1, globalThemeColor);
        MainWindow::_mwClsExtra.hNormalFont = CreateFontW(16, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
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
    RECT r = { x, y, x + w, y + MainWindow::_mwClsExtra.rowHeight };
    DrawTextW(WindowDC, text.c_str(), -1, &r, DT_SINGLELINE | DT_VCENTER | DT_RIGHT);
}

void MainWindow::DrawValueField(int x, int y, int w, const std::wstring& value, HDC WindowDC) {
    RECT r = { x, y, x + w, y + MainWindow::_mwClsExtra.rowHeight };

    auto oldBrush = SelectObject(WindowDC, GetStockObject(NULL_BRUSH));
    Rectangle(WindowDC, r.left, r.top, r.right, r.bottom);
    SelectObject(WindowDC, oldBrush);

    DrawTextW(WindowDC, value.c_str(), -1, &r, DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_END_ELLIPSIS);
}

void MainWindow::DrawTechCheckbox(int x, int y, int w, const std::wstring& name, bool checked, HDC WindowDC) {
    RECT rBox = { x, y + 4, x + 14, y + 18 };
    RECT rText = { x + 20, y, x + w, y + MainWindow::_mwClsExtra.rowHeight };

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

    const int startX = MainWindow::_mwClsExtra.startX;
    const int labelW = MainWindow::_mwClsExtra.labelW;
    const int rowH = MainWindow::_mwClsExtra.rowHeight;
    const int rightPad = MainWindow::_mwClsExtra.rightPad;
    const int gap = MainWindow::_mwClsExtra.gap;
    const int rightEdge = MainWindow::_mwClsExtra.rightEdge;

    int y = MainWindow::_mwClsExtra.startY;

    auto at = [&](int x0) -> MainWindow::Row { return MainWindow::Row{ WindowDC, x0, y, rightEdge, gap }; };

    at(startX).label(StringManager::GetStringByID(IDS_NAME), labelW).valueToEdge(gpu->GetDeviceName());
    y += rowH + 7;

    at(startX).label(StringManager::GetStringByID(IDS_REVISION), labelW)
        .value(gpu->GetRevision(), 50)
        .label(StringManager::GetStringByID(IDS_DEVICEID), 50)
        .value(gpu->GetDeviceID(), 160);
    y += rowH + 7;

    at(startX-5).label(StringManager::GetStringByID(IDS_BIOS_VERSION), labelW+5).valueToEdge(gpu->GetBIOSVersion());
    y += rowH + 7;

    at(startX).label(StringManager::GetStringByID(IDS_DRIVER_DATE), labelW)
        .value(gpu->GetDriverDate(), 70, 73 + gap)
        .label(StringManager::GetStringByID(IDS_DRIVER_VERSION), 73)
        .value(gpu->GetDriverVersion(), 115);
    y += rowH + 7;

    at(startX).label(StringManager::GetStringByID(IDS_BUS), labelW)
        .valueToEdge(gpu->GetBusCurrent() + L"/" + gpu->GetBusMaximum());
    y += rowH + 7;

    at(startX - 5).label(StringManager::GetStringByID(IDS_MEM_SIZE), labelW + 5)
        .value(std::to_wstring(gpu->GetMemSize()) + L" MB", 77, 73 + gap)
        .label(StringManager::GetStringByID(IDS_MEM_TYPE), 73)
        .value(gpu->GetMemoryType(), 115);
    y += rowH + 15;

    at(startX-5).label(StringManager::GetStringByID(IDS_GPU_CLOCK), labelW+5)
        .value(std::to_wstring(gpu->GetCoreClock()) + L" MHz", 77, 60)
        .label(StringManager::GetStringByID(IDS_MEM), 50, 53)
        .value(std::to_wstring(gpu->GetMemoryClock()) + L" MHz", 65, 35)
        .label(StringManager::GetStringByID(IDS_BOOST), 65, 68)
        .value(std::to_wstring(gpu->GetBoostCoreClock()) + L" MHz", 65);
    y += rowH + 7;

    at(startX-5).label(StringManager::GetStringByID(IDS_DEFAULT_CLOCK), labelW+5)
        .value(std::to_wstring(gpu->GetDefaultCoreClock()) + L" MHz", 77, 60)
        .label(StringManager::GetStringByID(IDS_MEM), 50, 53)
        .value(std::to_wstring(gpu->GetDefaultMemoryClock()) + L" MHz", 65, 35)
        .label(StringManager::GetStringByID(IDS_BOOST), 65, 68)
        .value(std::to_wstring(gpu->GetBoostMemoryClock()) + L" MHz", 65);
    y += rowH + 15;

    at(startX - 5).label(StringManager::GetStringByID(IDS_RESIZABLE_BAR), labelW + 5)
        .value(gpu->GetHasResizableBAR() ? L"Enable" : L"Disable", 77, 95 + gap)
        .label(StringManager::GetStringByID(IDS_DX_SUPPORT), 95)
        .value(gpu->GetDXMaxVersion(), 70);
    y += rowH + 30;

    at(startX - 20).label(StringManager::GetStringByID(IDS_SUPPORT), labelW)
        .check(L"OpenCL", labelW, gpu->GetHasOpenCL(), labelW + 5)
        .check(L"CUDA", 55, gpu->GetHasCUDA(), 55 + 7)
        .check(L"PhysX", 55, gpu->GetHasPhysX())
        .check(L"DirectCompute", 110, gpu->GetHasDirectCompute());
    y += rowH + 7;

    at(startX - 20 + labelW + gap)
        .check(L"Vulkan", labelW, gpu->GetHasVulkan(), labelW)
        .check(L"OpenGL 4.6", 85, gpu->GetHasVulkan(), 85)
        .check(L"DirectML", 67, gpu->GetHasDirectML(), 67 + 7)
        .check(L"RayTracing", 100, gpu->GetHasRayTracing());

    SelectObject(WindowDC, oldFont);
    SelectObject(WindowDC, oldPen);
}

void MainWindow::DrawSensors(HDC WindowDC) {

}

LRESULT WINAPI MainWindow::MainWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
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

        auto oldBrush = SelectObject(WindowDC, MainWindow::_mwClsExtra.hBrush);
        auto oldPen = SelectObject(WindowDC, MainWindow::_mwClsExtra.hPen);

        Rectangle(WindowDC, 5, TabHeight + 15, AppSizeX - 5, AppSizeY - ComboBoxHeight - 15);

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
    default: return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
    return 0;
}

void MainWindow::Release() {
    if (MainWindow::_wndClass) {
        DeleteObject(MainWindow::_mwClsExtra.hNormalFont);
        DeleteObject(MainWindow::_mwClsExtra.hBorderPen);
        DeleteObject(MainWindow::_mwClsExtra.hPen);
        DeleteObject(MainWindow::_mwClsExtra.hBrush);
        UnregisterClassW(L"GPUXWinClass", 0);
    }
}