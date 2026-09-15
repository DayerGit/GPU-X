#pragma once

#include <Windows.h>
#include <versionhelpers.h>

VERSIONHELPERAPI IsWindows11OrGreater() {
	OSVERSIONINFOEX osvi = { sizeof(osvi), 0, 0, 0, 0, {0}, 0, 0 };
	DWORDLONG const dwlConditionMask = VerSetConditionMask(VerSetConditionMask(VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL), VER_MINORVERSION, VER_GREATER_EQUAL), VER_BUILDNUMBER, VER_GREATER_EQUAL);

	osvi.dwMajorVersion = HIBYTE(_WIN32_WINNT_WIN10);
	osvi.dwMinorVersion = LOBYTE(_WIN32_WINNT_WIN10);
	osvi.dwBuildNumber = 22000;

	return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, dwlConditionMask) != FALSE;
}