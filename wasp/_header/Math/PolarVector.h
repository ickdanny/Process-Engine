#pragma once

#include "Angle.h"
#include "Vector2.h"

namespace wasp::math {
	
	class PolarVector {
	private:
		//fields
		float magnitude {};
		Angle angle {};
		
		Vector2 vector2Representation {};
	
	public:
		PolarVector() = default;
		
		PolarVector(float magnitude, Angle angle);
		
		PolarVector(const Vector2& vector2);
		
		//getters and setters
		float getMagnitude() const {
			return magnitude;
		}
		
		Angle getAngle() const {
			//return by value
			return angle;
		}
		
		void setMagnitude(float magnitude);
		
		void setAngle(Angle angle);
		
		//unary negative operator
		PolarVector operator-() const {
			return { magnitude, -angle };
		}
		
		//conversion to Vector2
		operator Vector2() const;
	
	private:
		void updateVector2Representation();
	};
	
	//helper function
	Angle getAngle(const Vector2& vector);
}