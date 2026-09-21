#pragma once

#include <Windows.h>

class MyButton {
public:
    MyButton() = delete;
    ~MyButton() = delete;

    static bool Init();
    static void Release();

private:
    struct ButtonClassExtra {
        HBRUSH Brushes[3];
        int currentActive;
    };
    static ButtonClassExtra _btnClsExtra;

    struct ButtonWindowExtra {
        wchar_t* windowName;
        HFONT hFont;
        bool isMouseInside;
        int currentBrush;
    };

    static ATOM _wndClass;
    static LRESULT WINAPI ButtonWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};