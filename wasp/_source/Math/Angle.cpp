#include "Math/Angle.h"

#include <cmath>

#include "Math/Constants.h"

namespace wasp::math {

	//getters
	float Angle::getAngleRadians() const {
		return toRadians(angle);
	}
	//setters
	void Angle::setAngle(float degrees) {
		this->angle = modAngle(degrees);
	}
	void Angle::setAngleRadians(float radians) {
		setAngle(toDegrees(radians));
	}

	float Angle::smallerDifference(const Angle& other) const {
		float difference{ angle - other.angle };
		if (difference > half) {
			return -(max - difference);
		}
		if (difference < -half) {
			return max + difference;
		}
		return difference;
	}

	float Angle::largerDifference(const Angle& other) const {
		float difference{ angle - other.angle };
		if (difference < half && difference >= min) {
			return -(max - difference);
		}
		if (difference > -half && difference <= min) {
			return max + difference;
		}
		return difference;
	}

	//returns the result of flipping this angle along the y axis
	Angle Angle::flipY() const {
		return { half - angle };
	}
	//returns the result of flipping this angle along the x axis
	Angle Angle::flipX() const {
		return { -angle };
	}

	//unary negative operator
	Angle Angle::operator-() const {
		return { half + angle };
	}

	//mathematical assignment operators
	Angle& Angle::operator+=(float right) {
		angle += right;
		angle = modAngle(angle);
		return *this;
	}
	Angle& Angle::operator-=(float right) {
		angle -= right;
		angle = modAngle(angle);
		return *this;
	}

	Angle::operator std::string() const {
		return std::to_string(angle);
	}

	//mathematical operators for Angle class
	bool operator==(const Angle& a, const Angle& b) {
		return a.getAngle() == b.getAngle();
	}
	bool operator!=(const Angle& a, const Angle& b) {
		return a.getAngle() != b.getAngle();
	}
	Angle operator+(const Angle& a, const Angle& b) {
		return { a.getAngle() + b.getAngle() };
	}
	Angle operator-(const Angle& a, const Angle& b) {
		return { a.getAngle() - b.getAngle() };
	}
}