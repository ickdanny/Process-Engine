#pragma once

namespace wasp::math {
	struct Rectangle {
		float x{};
		float y{};
		float width{};
		float height{};

		Rectangle(float x, float y, float width, float height)
			: x{ x }
			, y{ y }
			, width{ width }
			, height{ height } {
		}
	};
}