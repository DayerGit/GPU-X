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

#include "StringManager.h"
#include "DPIManager.h"

#include "resource.h"

int main() {
    DPIManager::Init();
    StringManager::ReadStrings(18);

    GetCurrentTheme();

    GPUFactory factory;

    MyButton::Init();
    MyComboBox::Init();
    MainWindow::Init(factory.LetsCreateGPUs());

    int screenSizeX = GetSystemMetrics(SM_CXFULLSCREEN);
    int screenSizeY = GetSystemMetrics(SM_CYFULLSCREEN);

    HWND Window = CreateWindowExW(WS_EX_DLGMODALFRAME, L"GPUXWinClass", L"GPU-X", WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 
        (screenSizeX - AppSizeX) / 2, (screenSizeY - AppSizeY) / 2, DPIManager::Scale(AppSizeX), DPIManager::Scale(AppSizeY), 0, 0, 0, 0);

    HWND GraphicsCardTabButton = CreateWindowExW(0, L"GPUXButtonClass", StringManager::GetStringByID(IDS_GRAPHICS_CARD), WS_CHILD | WS_VISIBLE, 
        DPIManager::Scale(5), DPIManager::Scale(15), DPIManager::Scale(120), DPIManager::Scale(TabHeight), Window, GC_TAB, 0, 0);
    HRGN GraphicsCardTabRegion = CreateRoundRectRgn(0, 0, DPIManager::Scale(120), DPIManager::Scale(TabHeight + 10), TabRound, TabRound);
    SetWindowRgn(GraphicsCardTabButton, GraphicsCardTabRegion, TRUE);

    HWND SensorsTabButton = CreateWindowExW(0, L"GPUXButtonClass", StringManager::GetStringByID(IDS_SENSORS), WS_CHILD | WS_VISIBLE, 
        DPIManager::Scale(130), DPIManager::Scale(15), DPIManager::Scale(80), DPIManager::Scale(TabHeight), Window, S_TAB, 0, 0);
    HRGN SensorsTabRegion = CreateRoundRectRgn(0, 0, DPIManager::Scale(80), DPIManager::Scale(TabHeight + 10), TabRound, TabRound);
    SetWindowRgn(SensorsTabButton, SensorsTabRegion, TRUE);

    HWND CardsListComboBox = CreateWindowExW(0, L"GPUXComboBox", L"CardList", WS_CHILD | WS_VISIBLE, DPIManager::Scale(5), 
        DPIManager::Scale(AppSizeY - ComboBoxHeight - 10), DPIManager::Scale(AppSizeX - 10), DPIManager::Scale(ComboBoxHeight), Window, CL_COMBOBOX, 0, 0);
    MainWindow::InitCardList(CardsListComboBox);

    HFONT hBoldFont = CreateFontW(DPIManager::Scale(20), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, 
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT hNormalFont = CreateFontW(DPIManager::Scale(20), 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE, 
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT hControlFont = NULL;

    SetWindowFont(GraphicsCardTabButton, hBoldFont, 1);
    SetWindowFont(SensorsTabButton, hBoldFont, 1);
    SetWindowFont(CardsListComboBox, hNormalFont, 1);
    
    if (IsWindows10OrGreater()) {
        HWND CloseButton = CreateWindowExW(0, L"GPUXButtonClass", L"\uE106", WS_CHILD | WS_VISIBLE, DPIManager::Scale(AppSizeX - 45), 
            0, DPIManager::Scale(45), DPIManager::Scale(32), Window, CLOSE_BUTTON, 0, 0);
        hControlFont = CreateFontW(DPIManager::Scale(10), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe MDL2 Assets");

        SetWindowFont(CloseButton, hControlFont, 1);
    }

    ShowWindow(Window, SW_SHOWNORMAL);

    MSG msg;
    while (GetMessageW(&msg, 0, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    DeleteObject(hNormalFont);
    DeleteObject(hBoldFont);
    if(hControlFont)
        DeleteObject(hControlFont);

    return 0;
}