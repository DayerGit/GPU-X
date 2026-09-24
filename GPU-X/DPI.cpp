#include <shellscalingapi.h>
#include "DPIManager.h"

UINT DPIManager::_currentDPI;

int DPIManager::Scale(int value) {
	return MulDiv(value, DPIManager::_currentDPI, 96);
}

void DPIManager::Init() {
    SetProcessDPIAware();

    UINT dpiX = 96;
    UINT dpiY = 96;

    HMODULE hShcore = LoadLibraryW(L"shcore.dll");
    if (hShcore) {
        typedef HRESULT(WINAPI* GetDpiForMonitorProc)(HMONITOR, int, UINT*, UINT*);
        GetDpiForMonitorProc pGetDpiForMonitor = (GetDpiForMonitorProc)GetProcAddress(hShcore, "GetDpiForMonitor");

        if (pGetDpiForMonitor) {
            HMONITOR hMonitor = MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);
            pGetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
        }
        FreeLibrary(hShcore);
    }
    else {
        HDC hdc = GetDC(NULL);
        if (hdc) {
            dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
            ReleaseDC(NULL, hdc);
        }
    }

    DPIManager::_currentDPI = dpiX;
}