#pragma once

#include "../Engine/Component.h"

namespace gm
{
	class Transform;
	class Input;

	class PlayerMovement : public Component
	{
	public:
		virtual void	OnInitialize() override;
		virtual void	OnUpdate() override;

		bool			IsMoving() const { return _isMoving; }

	private:
		Transform*		_ownerTransform;
		float			_moveSpeed = 600.f;
		bool			_isMoving = false;
	};
}
