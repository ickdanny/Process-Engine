#pragma once

#include <cmath>

#include "Point2.h"
#include "MathUtil.h"

namespace wasp::math {
	struct Vector2 {
		float x {};
		float y {};
		
		constexpr Vector2() = default;
		
		constexpr Vector2(float x, float y)
			: x { x }, y { y } {
		}
		
		//unary negative operator
		constexpr Vector2 operator-() const {
			return { -x, -y };
		}
		
		//conversion to point
		constexpr explicit operator Point2() const {
			return Point2 { x, y };
		}
		
		//conversion to string
		explicit operator std::string() const;
		
		//mathematical assignment operators
		
		//vector addition and subtraction
		Vector2& operator+=(const Vector2& right);
		
		Vector2& operator-=(const Vector2& right);
		
		//scalar multiplication and division
		Vector2& operator*=(float scalar);
		
		Vector2& operator/=(float scalar);
	};
	
	//mathematical operators for Vector2 class
	
	//vector addition and subtraction
	constexpr Vector2 operator+(const Vector2& a, const Vector2& b) {
		return { a.x + b.x, a.y + b.y };
	}
	
	constexpr Vector2 operator-(const Vector2& a, const Vector2& b) {
		return { a.x - b.x, a.y - b.y };
	}
	
	//point addition and subtraction
	constexpr Point2 operator+(const Point2& a, const Vector2& b) {
		return { a.x + b.x, a.y + b.y };
	}
	
	constexpr Point2 operator-(const Point2& a, const Vector2& b) {
		return { a.x - b.x, a.y - b.y };
	}
	
	//scalar multiplication and division
	constexpr Vector2 operator*(const Vector2& vector, float scalar) {
		return { vector.x * scalar, vector.y * scalar };
	}
	
	constexpr Vector2 operator*(float scalar, const Vector2& vector) {
		return { vector.x * scalar, vector.y * scalar };
	}
	
	constexpr Vector2 operator/(const Vector2& vector, float scalar) {
		throwIfZero(scalar, "vector divide by zero!");
		return { vector.x / scalar, vector.y / scalar };
	}
	
	constexpr Vector2 operator/(float scalar, const Vector2& vector) {
		throwIfZero(scalar, "vector divide by zero!");
		return { vector.x / scalar, vector.y / scalar };
	}
	
	//utility functions
	
	float getMagnitude(const Vector2& vector);
	
	//Returns a vector which, when added to A, will yield B.
	constexpr Vector2 vectorFromAToB(const Point2& a, const Point2& b) {
		return { b.x - a.x, b.y - a.y };
	}
	
	//not implementing equivalence operators due to floating point imprecision
}