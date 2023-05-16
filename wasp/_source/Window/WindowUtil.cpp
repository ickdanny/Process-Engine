#include "Window\WindowUtil.h"

#include <stdexcept>

namespace wasp::window {
	MONITORINFO getPrimaryMonitorInfo() {
		MONITORINFO toRet {};
		toRet.cbSize = sizeof(toRet);//needed
		BOOL success { GetMonitorInfo(getPrimaryMonitorHandle(), &toRet) };
		if( !success ) {
			throw std::runtime_error { "Error retrieving primary monitor info" };
		}
		return toRet;
	}
	
	HMONITOR getPrimaryMonitorHandle() {
		static constexpr POINT origin { 0, 0 };
		return MonitorFromPoint(origin, MONITOR_DEFAULTTOPRIMARY);
	}
	
	MONITORINFO getMonitorInfoFromWindow(HWND windowHandle) {
		MONITORINFO toRet {};
		toRet.cbSize = sizeof(toRet);//needed
		BOOL success {
			GetMonitorInfo(getMonitorHandleFromWindow(windowHandle), &toRet)
		};
		if( !success ) {
			throw std::runtime_error { "Error retrieving monitor info" };
		}
		return toRet;
	}
	
	HMONITOR getMonitorHandleFromWindow(HWND windowHandle) {
		return MonitorFromWindow(windowHandle, MONITOR_DEFAULTTONEAREST);
	}
	
	int getWindowBorderWidthPadding() {
		return 2 * (
			GetSystemMetrics(SM_CXBORDER)
				+ GetSystemMetrics(SM_CXFIXEDFRAME)
				+ GetSystemMetrics(SM_CXSIZEFRAME)
		);
	}
	
	int getWindowBorderHeightPadding() {
		return GetSystemMetrics(SM_CYCAPTION) + (2 * (
			GetSystemMetrics(SM_CYBORDER)
				+ GetSystemMetrics(SM_CYFIXEDFRAME)
				+ GetSystemMetrics(SM_CYSIZEFRAME)
		));
	}
	
	std::pair<int, int> fitToFullscreen() {
		MONITORINFO primaryMonitorInfo { getPrimaryMonitorInfo() };
		RECT& primaryMonitorRect { primaryMonitorInfo.rcMonitor };
		int width { primaryMonitorRect.right - primaryMonitorRect.left };
		int height { primaryMonitorRect.bottom - primaryMonitorRect.top };
		return { width, height };
	}
	
	std::pair<int, int> findCenterPositionWithBorder(const std::pair<int, int> size) {
		MONITORINFO primaryMonitorInfo { getPrimaryMonitorInfo() };
		RECT& primaryMonitorRect { primaryMonitorInfo.rcMonitor };
		int xCenter {
			(primaryMonitorRect.right - primaryMonitorRect.left) / 2
		};
		int yCenter {
			(primaryMonitorRect.bottom - primaryMonitorRect.top) / 2
		};
		int realWindowWidth {
			size.first + getWindowBorderWidthPadding()
		};
		int realWindowHeight {
			size.second + getWindowBorderHeightPadding()
		};
		const int x { xCenter - (realWindowWidth / 2) };
		const int y { yCenter - (realWindowHeight / 2) };
		return { x, y };
	}
	
	std::pair<int, int> findFullscreenPosition(const std::pair<int, int> size) {
		MONITORINFO primaryMonitorInfo { getPrimaryMonitorInfo() };
		RECT& primaryMonitorRect { primaryMonitorInfo.rcMonitor };
		return { primaryMonitorRect.left, primaryMonitorRect.top };
	}
}