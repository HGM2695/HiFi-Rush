#pragma once
#include <cmath>

namespace gm::math
{
	struct Vector2
	{
		float _x{};
		float _y{};

		Vector2() = default;
		Vector2(float x, float y) : _x(x), _y(y) {}

		bool operator==(const Vector2& rhs) const
		{
			constexpr float epsilon = 0.0001f;
			return std::fabs(_x - rhs._x) < epsilon && std::fabs(_y - rhs._y) < epsilon;
		}

		bool operator!=(const Vector2& rhs) const
		{
			return (*this == rhs) == false;
		}

		Vector2 operator+(const Vector2& rhs) const { return { _x + rhs._x, _y + rhs._y }; }
		Vector2 operator-(const Vector2& rhs) const { return { _x - rhs._x, _y - rhs._y }; }
		Vector2 operator*(float s) const { return { _x * s, _y * s }; }

		Vector2& operator+=(const Vector2& rhs) { _x += rhs._x; _y += rhs._y; return *this; }
		Vector2& operator-=(const Vector2& rhs) { _x -= rhs._x; _y -= rhs._y; return *this; }
		Vector2& operator*=(float s) { _x *= s; _y *= s; return *this; }

		float Length() const { return std::sqrt(_x * _x + _y * _y); }
		float LengthSquared() const { return _x * _x + _y * _y; }

		void Normalize()
		{
			float len = Length();
			if (len > 0.f)
			{
				_x /= len;
				_y /= len;
			}
		}

		Vector2 Normalized() const
		{
			float len = Length();
			if (len > 0.f)
				return { _x / len, _y / len };

			return { 0.f, 0.f };
		}

		float			Dot(const Vector2& rhs) const { return _x * rhs._x + _y * rhs._y; }

		static Vector2	Lerp(const Vector2& lhs, const Vector2& rhs, float t) { return lhs + (rhs - lhs) * t; }
		static float	Distance(const Vector2& a, const Vector2& b) { return (a - b).Length(); }
		static Vector2	Zero() { return { 0.f, 0.f }; }
	};

	inline Vector2 operator*(float s, const Vector2& v) { return { v._x * s, v._y * s };}
}
