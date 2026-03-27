#include "PlayerMovement.h"
#include "../Engine/Application.h"
#include "../Engine/GameObject.h"
#include "../Engine/GMAssert.h"
#include "../Engine/Input.h"
#include "../Engine/Rigidbody2D.h"
#include "../Engine/Transform.h"

namespace gm
{
	void PlayerMovement::OnInitialize()
	{
		_ownerTransform = GetOwner().GetTransform();
		GM_ASSERT(_ownerTransform, "PlayerMovement는 Transform이 필요합니다.");

		_ownerRigidbody = GetOwner().GetRigidbody2D();
		GM_ASSERT(_ownerRigidbody, "PlayerMovement는 Rigidbody2D가 필요합니다.");
	}

	void PlayerMovement::OnUpdate()
	{
		auto& input = APPLICATION.GetInput();
		const float moveAxisX = input.GetMoveAxisX();
		const bool isGrounded = _ownerRigidbody->IsGrounded();
		const float moveSpeed = isGrounded ? _groundMoveSpeed : _airMoveSpeed;
		math::Vector2 velocity = _ownerRigidbody->GetVelocity();

		_isMoving = moveAxisX != 0.f;

		if (moveAxisX < 0.f)
			_ownerTransform->SetForward({ -1.f, 0.f });
		else if (moveAxisX > 0.f)
			_ownerTransform->SetForward({ 1.f, 0.f });

		velocity._x = moveAxisX * moveSpeed;
		_ownerRigidbody->SetVelocity(velocity);

		if (isGrounded && input.IsKeyRepeat(KeyCode::T))
			_ownerRigidbody->AddImpulse({ 0.f, _jumpImpulse });
	}
}
