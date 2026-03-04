#pragma once

namespace gm::math
{
	struct Vector2
	{
		float _X{};
		float _Y{};

		constexpr Vector2() = default;
		constexpr Vector2(float x, float y) : _X(x), _Y(y) {}

		Vector2 operator+(const Vector2& rhs) const
		{
			return { _X + rhs._X, _Y + rhs._Y };
		}

		Vector2 operator-(const Vector2& rhs) const
		{
			return { _X - rhs._X, _Y - rhs._Y };
		}

		Vector2 operator*(float s) const
		{
			return { _X * s, _Y * s };
		}


	};
}