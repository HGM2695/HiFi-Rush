#pragma once

#include "../Engine/Component.h"

namespace gm
{
	class Transform;
	class Input;

	class PlayerMovement : public Component
	{
	public:
		virtual void OnInitialize() override;
		virtual void OnUpdate() override;

	private:
		Transform*		_transform;
		float			_moveSpeed = 100.f;
	};
}