#pragma once
#include <Windows.h>

#include "GPUFactory.h"
#include "Globals.h"
#include "DPIManager.h"

class MainWindow {
public:
	MainWindow() = delete;
	~MainWindow() = delete;

	static bool Init(std::vector<std::unique_ptr<GPU>>&& vectorOfGPUs);
	static void InitCardList(HWND hwnd);
	static void Release();

private:
	struct MainWindowClassExtra {
		HBRUSH hBrush, hCurrentThemeBrush;
		HPEN hPen, hBorderPen;
		HFONT hNormalFont;
		int currentTab;

		const int startX = 15;
		const int startY = TabHeight + 25;

		const int rowHeight = 22;

		const int labelW = 65;
		const int rightPad = 30;
		const int gap = 10;
		const int correction = 5;
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
		template <typename T>
		Row& graphic(int w, T* history, T min, T max, int advance = -1) {
			MainWindow::DrawGraphic(x, y, w, history, min, max, dc);
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

	template <typename T>
	static void DrawGraphic(int x, int y, int w, T* history, T min, T max, HDC WindowDC) {
		int rowHeight = DPIManager::Scale(MainWindow::_mwClsExtra.rowHeight);
		RECT r = { x, y, x + w, y + rowHeight };

		auto oldBrush = SelectObject(WindowDC, GetStockObject(NULL_BRUSH));
		Rectangle(WindowDC, r.left, r.top, r.right, r.bottom);

		int columnWidth = w / GPUX_HISTORY_DEPTH;
		int start = x + w;

		double range = static_cast<double>(max) - static_cast<double>(min);
		if (range <= 0.0) range = 1.0;

		SelectObject(WindowDC, MainWindow::_mwClsExtra.hCurrentThemeBrush);

		for (int i = 0; i < GPUX_HISTORY_DEPTH; i++) {
			T val = history[i];

			if (val < min) val = min;
			if (val > max) val = max;

			int normalizedY = r.bottom - static_cast<int>((static_cast<double>(val - min) * rowHeight) / range);

			Rectangle(WindowDC, start - columnWidth, normalizedY, start, r.bottom);
			start -= columnWidth;
		}

		SelectObject(WindowDC, oldBrush);
	}

	static void DrawGraphicsCard(HDC WindowDC);
	static void DrawSensors(HDC WindowDC);
	static LRESULT WINAPI MainWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};