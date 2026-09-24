#include <windowsx.h>

#include "ComboBox.h"
#include "WindowHelpers.h"
#include "VersionHelper.h"
#include "Themes.h"
#include "Globals.h"
#include "DPIManager.h"


ATOM MyComboBox::_wndClass;
MyComboBox::ComboBoxClassExtra MyComboBox::_cbbClsExtra;

bool MyComboBox::Init() {
    MyComboBox::_wndClass = CreateWindowClassW(L"GPUXComboBox", MyComboBox::ComboBoxWindowProc, GetStockBrush(BLACK_BRUSH),
        IDC_ARROW, 0, sizeof(MyComboBox::ComboBoxWindowExtra*));

    if (MyComboBox::_wndClass) {
        MyComboBox::_cbbClsExtra.brush = CreateSolidBrush(currentTheme.comboBoxColor);
        MyComboBox::_cbbClsExtra.pen = CreatePen(PS_SOLID, 1, currentTheme.comboBoxColor);
        MyComboBox::_cbbClsExtra.themeColorBrush = CreateSolidBrush(globalThemeColor);
        MyComboBox::_cbbClsExtra.font = CreateFontW(DPIManager::Scale(8), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe MDL2 Assets");

        if (!MyComboBox::_cbbClsExtra.font) {
            MyComboBox::_cbbClsExtra.font = CreateFontW(DPIManager::Scale(8), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        }
    }

    return MyComboBox::_wndClass != 0;
}

void MyComboBox::UpdateWindowPosAndRegion(HWND hwnd, int X, int Y, int cx, int cy, MyComboBox::ComboBoxWindowExtra* windowInfo) {
    SetWindowPos(hwnd, NULL, X, Y, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);

    HRGN ComboBoxRegion = CreateRoundRectRgn(0, 0, cx, cy, ComboBoxRound, ComboBoxRound);
    SetWindowRgn(hwnd, ComboBoxRegion, TRUE);
    InvalidateRect(hwnd, NULL, TRUE);
}

void MyComboBox::Minimize(HWND hwnd, MyComboBox::ComboBoxWindowExtra* windowInfo, bool needUpdateCursorPos, LPARAM lparam) {
    RECT thisRect = { 0 };
    HWND hParent = GetParent(hwnd);
    GetWindowRect(hwnd, &thisRect);

    int cx = thisRect.right - thisRect.left;

    POINT topLeft = { thisRect.left, thisRect.top };
    ScreenToClient(hParent, &topLeft);

    POINT downRight = { thisRect.right, thisRect.bottom };
    ScreenToClient(hParent, &downRight);

    int X = topLeft.x;
    int Y = downRight.y - DPIManager::Scale(ComboBoxHeight);
    int cy = DPIManager::Scale(ComboBoxHeight);

    if (needUpdateCursorPos) {
        int clickY = GET_Y_LPARAM(lparam);
        windowInfo->currentIndex = clickY / DPIManager::Scale(ComboBoxHeight);
    }

    windowInfo->currentState = MyComboBox::ComboBoxState::Minimize;

    MyComboBox::UpdateWindowPosAndRegion(hwnd, X, Y, cx, cy, windowInfo);

    SendMessageW(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(0, CBN_SELCHANGE), 0);
}

LRESULT WINAPI MyComboBox::ComboBoxWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

    auto windowInfo = (MyComboBox::ComboBoxWindowExtra*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    switch (msg) {
    case WM_CREATE: {

        MyComboBox::ComboBoxWindowExtra* ccbi = new MyComboBox::ComboBoxWindowExtra;
        ccbi->currentIndex = 0;
        ccbi->currentState = MyComboBox::ComboBoxState::Minimize;

        RECT wndClient = { 0 };
        GetClientRect(hwnd, &wndClient);

        HRGN ComboBoxRegion = CreateRoundRectRgn(0, 0, wndClient.right, DPIManager::Scale(ComboBoxHeight), ComboBoxRound, ComboBoxRound);
        SetWindowRgn(hwnd, ComboBoxRegion, TRUE);

        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)ccbi);
        break;
    }
    case WM_ERASEBKGND: return 1;
    case WM_PAINT: {
        PAINTSTRUCT PS;
        HDC WindowDC = BeginPaint(hwnd, &PS);

        RECT rcClient;
        GetClientRect(hwnd, &rcClient);
        int windowWidth = rcClient.right - rcClient.left;
        int windowHeight = rcClient.bottom - rcClient.top;

        HDC memDC = CreateCompatibleDC(WindowDC);
        HBITMAP memBitmap = CreateCompatibleBitmap(WindowDC, windowWidth, windowHeight);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

        FillRect(memDC, &rcClient, GetStockBrush(BLACK_BRUSH));

        auto oldBrush = SelectObject(memDC, MyComboBox::_cbbClsExtra.brush);
        auto oldPen = SelectObject(memDC, MyComboBox::_cbbClsExtra.pen);

        Rectangle(memDC, 0, 0, windowWidth, windowHeight);

        SetTextColor(memDC, currentTheme.standartWhiteColor);
        SetBkMode(memDC, TRANSPARENT);

        if (windowInfo && !windowInfo->strings.empty()) {
            SelectObject(memDC, windowInfo->hFont);

            if (windowInfo->currentState == MyComboBox::ComboBoxState::Minimize) {
                RECT rcText = rcClient;
                rcText.left += DPIManager::Scale(indentComboBox);
                rcText.right -= DPIManager::Scale(indentComboBox);

                DrawTextW(memDC, windowInfo->strings[windowInfo->currentIndex].c_str(), -1, &rcText, DT_SINGLELINE | DT_LEFT | DT_VCENTER);

                auto oldFont = SelectObject(memDC, MyComboBox::_cbbClsExtra.font);
                DrawTextW(memDC, IsWindows10OrGreater() ? L"\uE70D\0" : L"\u25BC", -1, &rcText, DT_SINGLELINE | DT_RIGHT | DT_VCENTER);
                SelectObject(memDC, oldFont);
            }
            else {
                int currentY = rcClient.bottom;

                for (int i = windowInfo->strings.size() - 1; i >= 0; i--) {
                    RECT rcItem = { rcClient.left + DPIManager::Scale(indentComboBox), currentY - DPIManager::Scale(ComboBoxHeight),
                        rcClient.right - DPIManager::Scale(indentComboBox), currentY };

                    if (i == windowInfo->mouseOn) {
                        auto oldBr = SelectObject(memDC, GetStockBrush(GRAY_BRUSH));
                        Rectangle(memDC, rcItem.left - DPIManager::Scale(indentComboBox),
                            rcItem.top, rcItem.right + DPIManager::Scale(indentComboBox), rcItem.bottom);
                        SelectObject(memDC, oldBr);
                    }

                    DrawTextW(memDC, windowInfo->strings[i].c_str(), -1, &rcItem, DT_SINGLELINE | DT_LEFT | DT_VCENTER);

                    if (i == windowInfo->currentIndex) {
                        auto oldBr = SelectObject(memDC, MyComboBox::_cbbClsExtra.themeColorBrush);
                        Rectangle(memDC, DPIManager::Scale(5), DPIManager::Scale(i * ComboBoxHeight + 10),
                            DPIManager::Scale(10), DPIManager::Scale(i * ComboBoxHeight + 25));
                        SelectObject(memDC, oldBr);
                    }

                    currentY -= DPIManager::Scale(ComboBoxHeight);
                }
            }
        }

        SelectObject(memDC, oldPen);
        SelectObject(memDC, oldBrush);

        BitBlt(WindowDC, 0, 0, windowWidth, windowHeight, memDC, 0, 0, SRCCOPY);

        SelectObject(memDC, oldBitmap);
        DeleteObject(memBitmap);
        DeleteDC(memDC);

        EndPaint(hwnd, &PS);
        break;
    }
    case WM_LBUTTONDOWN: {
        if (windowInfo && !windowInfo->strings.empty()) {
            int X = 0, Y = 0, cx = 0, cy = 0;

            SetFocus(hwnd);

            RECT parentRect = { 0 };
            RECT thisRect = { 0 };

            HWND hParent = GetParent(hwnd);
            GetClientRect(hParent, &parentRect);
            GetWindowRect(hwnd, &thisRect);
            cx = thisRect.right - thisRect.left;

            if (windowInfo->currentState == MyComboBox::ComboBoxState::Minimize) {
                int currentHeight = thisRect.bottom - thisRect.top;

                POINT topLeft = { thisRect.left, thisRect.top };
                ScreenToClient(hParent, &topLeft);

                int dropDownHeight = DPIManager::Scale(ComboBoxHeight) * (windowInfo->strings.size() - 1);
                X = topLeft.x;
                Y = topLeft.y - dropDownHeight;
                cy = dropDownHeight + currentHeight;

                windowInfo->currentState = MyComboBox::ComboBoxState::Maximize;

                MyComboBox::UpdateWindowPosAndRegion(hwnd, X, Y, cx, cy, windowInfo);
            }
            else MyComboBox::Minimize(hwnd, windowInfo, true, lparam);
        }
        break;
    }
    case WM_MOUSEMOVE: {
        if (windowInfo) {
            int curY = GET_Y_LPARAM(lparam);
            if (curY / DPIManager::Scale(ComboBoxHeight) != windowInfo->mouseOn) {
                windowInfo->mouseOn = curY / DPIManager::Scale(ComboBoxHeight);
                InvalidateRect(hwnd, 0, 0);
            }
        }
        break;
    }
    case WM_KILLFOCUS: {
        if (windowInfo && windowInfo->currentState == MyComboBox::ComboBoxState::Maximize) 
            MyComboBox::Minimize(hwnd, windowInfo, false, lparam);

        break;
    }
    case WM_SETFONT: {
        if (windowInfo) windowInfo->hFont = (HFONT)wparam;
        if (lparam) InvalidateRect(hwnd, 0, FALSE);
        break;
    }
    case CB_ADDSTRING: {
        if (windowInfo)
            windowInfo->strings.emplace_back((wchar_t*)lparam);
        break;
    }
    case CB_GETCURSEL: {
        return windowInfo->currentIndex;
    }
    case WM_DESTROY: {
        if (windowInfo)
            delete windowInfo;

        break;
    }
    default: return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
    return 0;
}

void MyComboBox::Release() {
    if (MyComboBox::_wndClass) {
        DeleteObject(MyComboBox::_cbbClsExtra.font);
        DeleteObject(MyComboBox::_cbbClsExtra.themeColorBrush);
        DeleteObject(MyComboBox::_cbbClsExtra.brush);
        UnregisterClassW(L"GPUXComboBox", 0);
    }
}