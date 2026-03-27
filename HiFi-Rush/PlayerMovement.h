#pragma once

#include "../Engine/Component.h"

namespace gm
{
	class Transform;
	class Rigidbody2D;
	class Input;

	class PlayerMovement : public Component
	{
	public:
		virtual void	OnInitialize() override;
		virtual void	OnUpdate() override;

		bool			IsMoving() const { return _isMoving; }

	private:
		Transform*		_ownerTransform = nullptr;
		Rigidbody2D*	_ownerRigidbody = nullptr;
		float			_groundMoveSpeed = 400.f;
		float			_airMoveSpeed = 250.f;
		float			_jumpImpulse = 700.f;
		bool			_isMoving = false;
	};
}
