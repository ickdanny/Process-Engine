#pragma once

#include <stdexcept>

#include "Constants.h"

namespace wasp::math {
	
	constexpr void throwIfZero(int i, const std::string& message = "int is zero!") {
		if( i == 0 ) {
			throw std::runtime_error { message };
		}
	}
	
	constexpr void throwIfZero(float f, const std::string& message = "float is zero!") {
		if( f == 0.0f ) {
			throw std::runtime_error { message };
		}
	}
	
	template <typename T>
	constexpr T ceilingIntegerDivide(T x, T y) {
		static_assert(std::is_integral<T>::value, "integral type required!");
		return x / y + (x % y != 0);
	}
	
	constexpr float toRadians(float degrees) {
		constexpr float ratio { pi / 180.0f };
		return degrees * ratio;
	}
	
	constexpr float toDegrees(float radians) {
		constexpr float ratio { 180.0f / pi };
		return radians * ratio;
	}
}