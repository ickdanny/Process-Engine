#include "Math/PolarVector.h"

#include <cmath>

namespace wasp::math {
	
	PolarVector::PolarVector(float magnitude, Angle angle)
		: magnitude { magnitude }
		, angle { angle } {
		updateVector2Representation();
	}
	
	PolarVector::PolarVector(const Vector2& vector)
		: magnitude { math::getMagnitude(vector) }
		, angle { math::getAngle(vector) }
		, vector2Representation { vector } {
	}
	
	void PolarVector::setMagnitude(float magnitude) {
		this->magnitude = magnitude;
		updateVector2Representation();
	}
	
	void PolarVector::setAngle(Angle angle) {
		this->angle = angle;
		updateVector2Representation();
	}
	
	PolarVector::operator Vector2() const {
		return vector2Representation;
	}
	
	void PolarVector::updateVector2Representation() {
		float radians { angle.getAngleRadians() };
		vector2Representation.x = magnitude * std::cosf(radians);
		vector2Representation.y = -1 * magnitude * std::sinf(radians);
	}
	
	Angle getAngle(const Vector2& vector) {
		//atan2 takes (y, x) and not (x, y)
		return Angle { toDegrees(std::atan2f(-vector.y, vector.x)) };
	}
}