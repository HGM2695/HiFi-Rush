#pragma once

#include "Component.h"

namespace gm
{
	class TransformComponent;
	class Rigidbody2DComponent;
	class Input;

	class PlayerMovementComponent : public Component
	{
	public:
		virtual TickGroup GetTickGroup() const override { return TickGroup::Movement; }
		virtual void	OnInitialize() override;
		virtual void	OnTick(float deltaTime) override;

		bool			IsMoving() const { return _isMoving; }
		const Vector2&	GetFacingDirection() const { return _facingDirection; }
		bool			IsFacingLeft() const { return _facingDirection.x < 0.f; }
		bool			IsFacingRight() const { return _facingDirection.x > 0.f; }

	private:
		TransformComponent*		_ownerTransform = nullptr;
		Rigidbody2DComponent*	_ownerRigidbody = nullptr;
		Vector2			_facingDirection{ 1.f, 0.f };
		float			_groundMoveSpeed = 400.f;
		float			_airMoveSpeed = 250.f;
		float			_jumpImpulse = 700.f;
		bool			_isMoving = false;
	};
}
