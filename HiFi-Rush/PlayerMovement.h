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
		Transform*		_ownerTransform;
		float			_moveSpeed = 600.f;
	};
}