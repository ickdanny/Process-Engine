#pragma once

#include "AABB.h"
#include "Angle.h"
#include "Constants.h"
#include "Point2.h"
#include "PolarVector.h"
#include "Rectangle.h"
#include "Vector2.h"

namespace wasp::math {
	
	//master include file, as well as some utility functions
	
	//applying vectors to AABBs
	constexpr AABB operator+(const AABB& aabb, const Vector2& vector) {
		return {
			aabb.xLow + vector.x,
			aabb.xHigh + vector.x,
			aabb.yLow + vector.y,
			aabb.yHigh + vector.y
		};
	}
	
	constexpr AABB operator-(const AABB& aabb, const Vector2& vector) {
		return {
			aabb.xLow - vector.x,
			aabb.xHigh - vector.x,
			aabb.yLow - vector.y,
			aabb.yHigh - vector.y
		};
	}
	
	float distanceFromAToB(const Point2& a, const Point2& b);
	
	Angle getAngleFromAToB(const Point2& a, const Point2& b);
	
	//Returns 360/n
	constexpr float fullAngleDivide(int n) {
		return 360.0f / static_cast<float>(n);
	}
	
	bool isPointWithinAABB(const Point2& point, const AABB& aabb);
}