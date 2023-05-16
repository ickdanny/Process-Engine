#pragma once

#include <cmath>
#include <algorithm>

#include "Point2.h"

namespace wasp::math {
	struct AABB {
	public:
		//fields
		float xLow {};
		float xHigh {};
		float yLow {};
		float yHigh {};
		
		//constructors
		
		//Constructs a zero AABB
		constexpr AABB() = default;
		
		//Constructs a copy of the given AABB
		constexpr AABB(const AABB& toCopy) = default;
		
		//Constructs an AABB by moving the given AABB (will not invalidate)
		constexpr AABB(AABB&& toMove) = default;
		
		//Constructs an AABB with the given radius
		constexpr AABB(float radius)
			: xLow { -radius }, xHigh { radius }, yLow { -radius }, yHigh { radius } {
		}
		
		//Constructs a symmetrical AABB with the given x and y radii
		constexpr AABB(float x, float y)
			: xLow { -x }, xHigh { x }, yLow { -y }, yHigh { y } {
		}
		
		//Constructs an AABB with the specified dimensions
		constexpr AABB(float xLow, float xHigh, float yLow, float yHigh)
			: xLow { xLow }, xHigh { xHigh }, yLow { yLow }, yHigh { yHigh } {
		}
		
		//Assignment operators
		AABB& operator=(const AABB& toCopy) = default;
		
		AABB& operator=(AABB&& toMove) = default;
		
		//Non-trivial getters
		float getWidth() const {
			return std::abs(xHigh - xLow);
		}
		
		float getHeight() const {
			return std::abs(yHigh - yLow);
		}
		
		float getArea() const {
			return getWidth() * getHeight();
		}
		
		Point2 getCenter() const {
			return { (xLow + xHigh) / 2.0f, (yLow + yHigh) / 2.0f };
		}
		
		//Sets the x radius to the given value.
		constexpr void setX(float x) {
			xLow = -x;
			xHigh = x;
		}
		
		//Sets the y radius to the given value
		constexpr void setY(float y) {
			yLow = -y;
			yHigh = y;
		}
		
		//Returns the AABB centered at the given point with the same dimensions as this.
		constexpr AABB centerAt(const Point2& center) const {
			return {
				xLow + center.x,
				xHigh + center.x,
				yLow + center.y,
				yHigh + center.y
			};
		}
	};
	
	//utility functions
	
	//Returns true if the two given AABBs intersect
	constexpr bool collides(const AABB& left, const AABB& right) {
		return left.xLow <= right.xHigh
			&& left.xHigh >= right.xLow
			&& left.yLow <= right.yHigh
			&& left.yHigh >= right.yLow;
	}
	
	//Returns the smallest AABB which wholly contains both the given AABBs
	constexpr AABB makeEncompassingAABB(const AABB& left, const AABB& right) {
		return {
			std::min(left.xLow, right.xLow),
			std::max(left.xHigh, right.xHigh),
			std::min(left.yLow, right.yLow),
			std::max(left.yHigh, right.yHigh)
		};
	}
}