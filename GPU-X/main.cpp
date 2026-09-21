#include <iostream>

#include <Windows.h>
#include <Windowsx.h>

#include "VersionHelper.h"
#include "GPUFactory.h"
#include "CommonID.h"
#include "Globals.h"
#include "WindowHelpers.h"
#include "MainWindow.h"
#include "Button.h"
#include "ComboBox.h"
#include "DPI.h"

#include "StringManager.h"

#include "resource.h"

int main() {
    StringManager::ReadStrings(18);

    SetProcessDPIAware();
    GetCurrentTheme();

    GPUFactory factory;

    MyButton::Init();
    MyComboBox::Init();
    MainWindow::Init(factory.LetsCreateGPUs());

    int screenSizeX = GetSystemMetrics(SM_CXFULLSCREEN);
    int screenSizeY = GetSystemMetrics(SM_CYFULLSCREEN);

    HWND Window = CreateWindowExW(WS_EX_DLGMODALFRAME, L"GPUXWinClass", L"GPU-X", WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, (screenSizeX - AppSizeX) / 2, (screenSizeY - AppSizeY) / 2, AppSizeX, AppSizeY, 0, 0, 0, 0);

    HWND GraphicsCardTabButton = CreateWindowExW(0, L"GPUXButtonClass", StringManager::GetStringByID(IDS_GRAPHICS_CARD), WS_CHILD | WS_VISIBLE, 5, 15, 120, TabHeight, Window, GC_TAB, 0, 0);
    HRGN GraphicsCardTabRegion = CreateRoundRectRgn(0, 0, 120, TabHeight + 10, TabRound, TabRound);
    SetWindowRgn(GraphicsCardTabButton, GraphicsCardTabRegion, TRUE);

    HWND SensorsTabButton = CreateWindowExW(0, L"GPUXButtonClass", StringManager::GetStringByID(IDS_SENSORS), WS_CHILD | WS_VISIBLE, 130, 15, 80, TabHeight, Window, S_TAB, 0, 0);
    HRGN SensorsTabRegion = CreateRoundRectRgn(0, 0, 80, TabHeight + 10, TabRound, TabRound);
    SetWindowRgn(SensorsTabButton, SensorsTabRegion, TRUE);

    HWND CardsListComboBox = CreateWindowExW(0, L"GPUXComboBox", L"CardList", WS_CHILD | WS_VISIBLE, 5, AppSizeY - ComboBoxHeight - 10, AppSizeX - 10, ComboBoxHeight, Window, CL_COMBOBOX, 0, 0);
    MainWindow::InitCardList(CardsListComboBox);

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