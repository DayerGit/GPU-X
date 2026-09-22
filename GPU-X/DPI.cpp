#include <shellscalingapi.h>
#include "DPIManager.h"

UINT DPIManager::_currentDPI;

int DPIManager::Scale(int value) {
	return MulDiv(value, DPIManager::_currentDPI, 96);
}

void DPIManager::Init() {
	SetProcessDPIAware();
	UINT dpiX, dpiY;
	HMONITOR hMonitor = MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);
	HRESULT hRes = GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
	if (SUCCEEDED(hRes)) DPIManager::_currentDPI = dpiX;
	else DPIManager::_currentDPI = 96;
}