#include "Math/Geometry.h"

namespace wasp::math {

	float distanceFromAToB(const Point2& a, const Point2& b) {
		return getMagnitude(vectorFromAToB(a, b));
	}

	Angle getAngleFromAToB(const Point2& a, const Point2& b) {
		return getAngle(vectorFromAToB(a, b));
	}

	bool isPointWithinAABB(const Point2& point, const AABB& aabb) {
		return point.x > aabb.xLow
			&& point.x < aabb.xHigh
			&& point.y > aabb.yLow
			&& point.y < aabb.yHigh;
	}
}