#pragma once
#include <vector>
#include <string>

#include <Windows.h>


class MyComboBox {
public: 
    MyComboBox() = delete;
    ~MyComboBox() = delete;

    static bool Init();
    static void Release();

private:
    enum class ComboBoxState {
        Minimize = 0,
        Maximize = 1
    };

    struct ComboBoxClassExtra {
        HBRUSH brush, themeColorBrush;
        HPEN pen;
        HFONT font;
    };
    static ComboBoxClassExtra _cbbClsExtra;

    struct ComboBoxWindowExtra {
        int currentIndex, mouseOn;
        ComboBoxState currentState;
        std::vector<std::wstring> strings;
        HFONT hFont;
    };

    static ATOM _wndClass;

    static void UpdateWindowPosAndRegion(HWND hwnd, int X, int Y, int cx, int cy, MyComboBox::ComboBoxWindowExtra* windowInfo);
    static void Minimize(HWND hwnd, MyComboBox::ComboBoxWindowExtra* windowInfo, bool needUpdateCursorPos, LPARAM lparam);
    static LRESULT WINAPI ComboBoxWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};