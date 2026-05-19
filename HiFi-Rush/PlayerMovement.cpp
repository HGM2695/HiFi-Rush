#include "PlayerMovement.h"
#include "Application.h"
#include "GameObject.h"
#include "Input.h"
#include "Rigidbody2D.h"
#include "Transform.h"

namespace gm
{
	void PlayerMovement::OnInitialize()
	{
		_ownerTransform = GetOwner().GetTransform();
		GM_ASSERT(_ownerTransform, "PlayerMovement는 Transform이 필요합니다.");

		_ownerRigidbody = GetOwner().GetRigidbody2D();
		GM_ASSERT(_ownerRigidbody, "PlayerMovement는 Rigidbody2D가 필요합니다.");
	}

	void PlayerMovement::OnTick(float deltaTime)
	{
		auto& input = APPLICATION.GetInput();
		const float moveAxisX = input.GetMoveAxisX();
		const bool isGrounded = _ownerRigidbody->IsGrounded();
		const float moveSpeed = isGrounded ? _groundMoveSpeed : _airMoveSpeed;
		Vector2 velocity = _ownerRigidbody->GetVelocity();

		_isMoving = moveAxisX != 0.f;

		if (moveAxisX < 0.f)
			_facingDirection = { -1.f, 0.f };
		else if (moveAxisX > 0.f)
			_facingDirection = { 1.f, 0.f };

		velocity.x = moveAxisX * moveSpeed;
		_ownerRigidbody->SetVelocity(velocity);

		if (isGrounded && input.IsKeyRepeat(KeyCode::T))
			_ownerRigidbody->AddImpulse({ 0.f, _jumpImpulse });
	}
}
