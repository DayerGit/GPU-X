#pragma once
#include <Windows.h>

#include "GPUFactory.h"
#include "Globals.h"

class MainWindow {
public:
	MainWindow() = delete;
	~MainWindow() = delete;

	static bool Init(std::vector<std::unique_ptr<GPU>>&& vectorOfGPUs);
	static void InitCardList(HWND hwnd);
	static void Release();

private:
	struct MainWindowClassExtra {
		HBRUSH hBrush;
		HPEN hPen, hBorderPen;
		HFONT hNormalFont;
		int currentTab;

		const int startX = 15;
		const int startY = TabHeight + 25;

		const int rowHeight = 22;

		const int labelW = 65;
		const int rightPad = 30;
		const int gap = 10;
		const int rightEdge = AppSizeX - rightPad;
	};
	static MainWindowClassExtra _mwClsExtra;

	struct Row {
		HDC dc;
		int x, y, rightEdge, gap;

		Row& label(const std::wstring& t, int w, int advance = -1) {
			MainWindow::DrawLabel(x, y, w, t, dc);
			x += (advance < 0 ? w + gap : advance);
			return *this;
		}
		Row& value(const std::wstring& t, int w, int advance = -1) {
			MainWindow::DrawValueField(x, y, w, t, dc);
			x += (advance < 0 ? w + gap : advance);
			return *this;
		}
		Row& valueToEdge(const std::wstring& t) { return value(t, rightEdge - x); }
		Row& check(const std::wstring& t, int w, bool on, int advance = -1) {
			MainWindow::DrawTechCheckbox(x, y, w, t, on, dc);
			x += (advance < 0 ? w + gap : advance);
			return *this;
		}
	};

	static ATOM _wndClass;
	static HWND _curComboBox;

	static std::vector<std::unique_ptr<GPU>> _vectorOfGPUs;

	static void DrawLabel(int x, int y, int w, const std::wstring& text, HDC WindowDC);
	static void DrawValueField(int x, int y, int w, const std::wstring& value, HDC WindowDC);
	static void DrawTechCheckbox(int x, int y, int w, const std::wstring& name, bool checked, HDC WindowDC);

	static void DrawGraphicsCard(HDC WindowDC);
	static void DrawSensors(HDC WindowDC);
	static LRESULT WINAPI MainWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};