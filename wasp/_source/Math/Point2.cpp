#include "Math/Point2.h"

namespace wasp::math{
	Point2::operator std::string() const {
		return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
	}
}