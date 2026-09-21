#pragma once

#include <Windows.h>


struct Theme {
    COLORREF standartAppTheme = RGB(0, 120, 215);
    COLORREF standartWhiteColor = RGB(255, 255, 255);
    COLORREF comboBoxColor = RGB(80, 80, 80);
    COLORREF standartGreenColor = RGB(80, 210, 80);
    COLORREF standartGrayColor = RGB(45, 45, 45);
    COLORREF standartLightGrayColor = RGB(10, 10, 10);
    COLORREF standartBlackColor = RGB(0, 0, 0);
};

constexpr Theme currentTheme;
