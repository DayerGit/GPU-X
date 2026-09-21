#include "WindowHelpers.h"
#include "Themes.h"
#include "Globals.h"

#include <dwmapi.h>

ATOM CreateWindowClassW(LPCWSTR Name, WNDPROC ClassProc, HBRUSH BackgroundBrush, LPCWSTR Cursor, LPWSTR Icon, int extraWindow) {
    WNDCLASSW wcl;
    memset(&wcl, 0, sizeof(wcl));

    wcl.lpszClassName = Name;
    wcl.lpfnWndProc = ClassProc;
    wcl.hbrBackground = BackgroundBrush;
    wcl.hCursor = LoadCursorW(0, Cursor);
    wcl.cbWndExtra = extraWindow;
    wcl.hIcon = LoadIconW(GetModuleHandle(0), MAKEINTRESOURCEW(Icon));

    return RegisterClassW(&wcl);
}

void GetCurrentTheme() {
    DWORD rawDwmColor = 0;
    BOOL paid;

    HRESULT hRes = DwmGetColorizationColor(&rawDwmColor, &paid);

    if (SUCCEEDED(hRes)) {
        BYTE r = (rawDwmColor >> 16) & 0xFF;
        BYTE g = (rawDwmColor >> 8) & 0xFF;
        BYTE b = rawDwmColor & 0xFF;

        globalThemeColor = RGB(r, g, b);
    }
    else globalThemeColor = currentTheme.standartAppTheme;
}