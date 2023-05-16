#pragma once

#include <string>

namespace wasp::math {
	struct Point2 {
		//fields
		float x {};
		float y {};
		
		//Constructors
		
		//Constructs a point at (0, 0)
		constexpr Point2() = default;
		
		//Constructs a point at the given coordinates
		constexpr Point2(float x, float y)
			: x { x }, y { y } {
		}
		
		//Constructs a copy of the given point
		constexpr Point2(const Point2& toCopy) = default;
		
		//Constructs a point by moving the given point (does not invalidate)
		constexpr Point2(Point2&& toMove) = default;
		
		//Assignment operators
		Point2& operator=(const Point2& toCopy) = default;
		
		Point2& operator=(Point2&& toMove) = default;
		
		//Conversion to string
		explicit operator std::string() const;
	};
	
	//not implementing any operators due to floating point imprecision
}