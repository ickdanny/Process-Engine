#pragma once

#include <utility>

#include "windowsInclude.h"

namespace wasp::window {
	MONITORINFO getPrimaryMonitorInfo();
	HMONITOR getPrimaryMonitorHandle();

	MONITORINFO getMonitorInfoFromWindow(HWND windowHandle);
	HMONITOR getMonitorHandleFromWindow(HWND windowHandle);

	int getWindowBorderWidthPadding();
	int getWindowBorderHeightPadding();

	std::pair<int, int> fitToFullscreen();
	std::pair<int, int> findCenterPositionWithBorder(const std::pair<int, int> size);
	std::pair<int, int> findFullscreenPosition(const std::pair<int, int> size);
}