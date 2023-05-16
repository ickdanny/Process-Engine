#pragma once

#include "Window\WindowMode.h"
#include "Window\WindowUtil.h"
#include "MainConfig.h"

namespace process::game::windowmodes {
	
	//we use extern const because constexpr doesn't work with wide string literal
	
	extern const wasp::window::WindowMode windowed {
		L"WINDOWED",
		WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION,
		0,
		[]() -> std::pair<int, int> {
			return { config::windowWidth, config::windowHeight };
		},
		wasp::window::findCenterPositionWithBorder
	};
	
	extern const wasp::window::WindowMode fullscreen {
		L"FULLSCREEN",
		WS_POPUP | WS_VISIBLE,
		WS_EX_APPWINDOW,
		wasp::window::fitToFullscreen,
		wasp::window::findFullscreenPosition
	};
}