#pragma once
#include <Windows.h>

class DPIManager {
public:
	DPIManager() = delete;
	~DPIManager() = delete;

	static void Init();
	static int Scale(int value);

private:
	static UINT _currentDPI;
};
int ScaleForDPI(int value, UINT dpi);
UINT GetMonitorDPI(HMONITOR hMonitor);