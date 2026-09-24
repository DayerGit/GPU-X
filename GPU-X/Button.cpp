#include <windowsx.h>

#include "Button.h"
#include "Globals.h"
#include "WindowHelpers.h"
#include "Themes.h"

#include <iostream>

ATOM MyButton::_wndClass;
MyButton::ButtonClassExtra MyButton::_btnClsExtra;

bool MyButton::Init() {
	MyButton::_wndClass = CreateWindowClassW(L"GPUXButtonClass", MyButton::ButtonWindowProc, GetStockBrush(BLACK_BRUSH),
							IDC_ARROW, 0, sizeof(MyButton::ButtonWindowExtra*));
    if (MyButton::_wndClass) {
        MyButton::_btnClsExtra.Brushes[0] = CreateSolidBrush(currentTheme.standartGrayColor);
        MyButton::_btnClsExtra.Brushes[1] = CreateSolidBrush(currentTheme.standartBlackColor);
        MyButton::_btnClsExtra.Brushes[2] = CreateSolidBrush(currentTheme.standartLightGrayColor);
    }

    return MyButton::_wndClass != 0;
}

LRESULT WINAPI MyButton::ButtonWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

    auto windowInfo = (MyButton::ButtonWindowExtra*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    switch (msg) {
    case WM_CREATE: {

        MyButton::ButtonWindowExtra* cbi = new MyButton::ButtonWindowExtra;

        int len = GetWindowTextLengthW(hwnd) + 1;
        auto WindowName = new wchar_t[len];

        GetWindowTextW(hwnd, WindowName, len);

        cbi->windowName = WindowName;
        cbi->hFont = GetStockFont(DEFAULT_GUI_FONT);
        cbi->isMouseInside = false;
        cbi->currentBrush = 1;

        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)cbi);
        break;
    }
    case WM_ERASEBKGND: return 1;
    case WM_PAINT: {
        if (windowInfo) {
            PAINTSTRUCT PS;
            HDC WindowDC = BeginPaint(hwnd, &PS);

            RECT rcClient;
            GetClientRect(hwnd, &rcClient);

            const int Width = rcClient.right - rcClient.left;
            const int Height = rcClient.bottom - rcClient.top;

            HDC memDC = CreateCompatibleDC(WindowDC);
            HBITMAP memBitmap = CreateCompatibleBitmap(WindowDC, Width, Height);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

            FillRect(memDC, &rcClient, MyButton::_btnClsExtra.Brushes[MyButton::_btnClsExtra.currentActive == (int)GetMenu(hwnd) ? 0 : windowInfo->currentBrush]);

            auto oldFont = SelectObject(memDC, windowInfo->hFont);

            SetTextColor(memDC, currentTheme.standartWhiteColor);
            SetBkMode(memDC, TRANSPARENT);
            DrawTextW(memDC, windowInfo->windowName, -1, &rcClient, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

            SelectObject(memDC, oldFont);

            BitBlt(WindowDC, 0, 0, Width, Height, memDC, 0, 0, SRCCOPY);

            SelectObject(memDC, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(memDC);

            EndPaint(hwnd, &PS);
        }
        break;
    }
    case WM_MOUSEMOVE: {
        if (windowInfo && !windowInfo->isMouseInside) {
            windowInfo->isMouseInside = true;

            TRACKMOUSEEVENT leaveTME = { 0 };
            leaveTME.cbSize = sizeof(leaveTME);
            leaveTME.dwFlags = TME_LEAVE;
            leaveTME.hwndTrack = hwnd;
            TrackMouseEvent(&leaveTME);

            windowInfo->currentBrush = 2;
            InvalidateRect(hwnd, 0, 1);
        }
        break;
    }
    case WM_MOUSELEAVE: {
        if (windowInfo) {
            windowInfo->currentBrush = 1;
            windowInfo->isMouseInside = false;

            InvalidateRect(hwnd, 0, 1);
        }
        break;
    }
    case WM_LBUTTONDOWN: {
        if (windowInfo) {
            WPARAM msgWPARAM = (WPARAM)GetMenu(hwnd);

            windowInfo->currentBrush = 1;
            MyButton::_btnClsExtra.currentActive = msgWPARAM;

            SendMessageW(GetParent(hwnd), WM_COMMAND, (WPARAM)GetMenu(hwnd), (LPARAM)hwnd);
            SetFocus(hwnd);
        }
        break;
    }
    case WM_SETFONT: {
        if (windowInfo) windowInfo->hFont = (HFONT)wparam;
        if (lparam) InvalidateRect(hwnd, 0, FALSE);
        break;
    }
    case WM_DESTROY: {
        if (windowInfo) {
            delete[] windowInfo->windowName;
            delete windowInfo;
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        }
        break;
    }
    default: return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
    return 0;
}

void MyButton::Release() {
    if (MyButton::_wndClass) {
        for (int i = 0; i < ARRAYSIZE(MyButton::_btnClsExtra.Brushes); i++)
            DeleteObject(MyButton::_btnClsExtra.Brushes[i]);

        UnregisterClassW(L"GPUXButtonClass", 0);
    }
}