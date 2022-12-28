#pragma once

#include <string>
#include <variant>
#include <functional>

#include "windowsInclude.h"

namespace wasp::window {

	struct WindowMode {
		const std::wstring modeName{};

		const DWORD windowStyle{};
		const DWORD windowExtraStyle{};

		const std::function<std::pair<int, int>()> sizeFunction{};

		//gets passed size
		const std::function<std::pair<int, int>(const std::pair<int, int>)> positionFunction{};
	};
}