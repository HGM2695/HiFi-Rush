#pragma once
#include "Component.h"
#include "Vector2.h"

namespace gm
{
	class Transform : public Component
	{
	public:
		Transform();
		virtual ~Transform();

		void					SetPosition(const math::Vector2& position) { _position = position; }
		const math::Vector2&	GetPosition() { return _position; }
		float					GetX() { return _position._X; }
		float					GetY() { return _position._Y; }

	private:
		math::Vector2 _position;
	};
}