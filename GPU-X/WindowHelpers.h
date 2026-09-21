#pragma once
#include <Windows.h>

ATOM CreateWindowClassW(LPCWSTR Name, WNDPROC ClassProc, HBRUSH BackgroundBrush, LPCWSTR Cursor, LPWSTR Icon, int extraWindow);
void GetCurrentTheme();