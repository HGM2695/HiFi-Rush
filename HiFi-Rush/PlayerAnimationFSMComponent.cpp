#include "PlayerAnimationFSMComponent.h"
#include "AnimatedSpriteComponent.h"
#include "GameObject.h"
#include "PlayerMovementComponent.h"
#include "SpriteAnimator.h"
#include <windows.h>

namespace gm
{
	void PlayerAnimationFSMComponent::OnInitialize()
	{
		_animatedSpriteComponent = GetOwner().GetComponent<AnimatedSpriteComponent>();
		GM_ASSERT(_animatedSpriteComponent, "PlayerAnimationFSM은 AnimatedSpriteComponent가 필요합니다.");

		SpriteAnimator& animator = _animatedSpriteComponent->GetAnimator();
		_notifyConnection = animator.BindNotifyListener(
			[this](const std::wstring& notifyName)
			{
				OnAnimationNotify(notifyName);
			});

		_playerMovement = GetOwner().GetComponent<PlayerMovementComponent>();
		GM_ASSERT(_playerMovement, "PlayerAnimationFSM은 PlayerMovement가 필요합니다.");
		_lastFacingDirection = _playerMovement->GetFacingDirection();

		ChangeState(PlayerAnimState::Idle);
	}

	void PlayerAnimationFSMComponent::OnTick(float deltaTime)
	{
		SyncDirection();
		UpdateState();
	}

	void PlayerAnimationFSMComponent::SyncDirection()
	{
		const Vector2 facingDirection = _playerMovement->GetFacingDirection();
		if (_lastFacingDirection == facingDirection)
			return;

		_lastFacingDirection = facingDirection;
		PlayCurrentAnimation({ _animatedSpriteComponent->GetAnimator().GetPlayTime() });
	}

	void PlayerAnimationFSMComponent::UpdateState()
	{
		if (_playerMovement->IsMoving())
			ChangeState(PlayerAnimState::Move);
		else
			ChangeState(PlayerAnimState::Idle);
	}

	void PlayerAnimationFSMComponent::ChangeState(PlayerAnimState nextState)
	{
		if (_currentState == nextState)
			return;

		_currentState = nextState;
		PlayCurrentAnimation();
	}

	void PlayerAnimationFSMComponent::PlayCurrentAnimation(const AnimationPlayOption& playOption)
	{
		SpriteAnimator& animator = _animatedSpriteComponent->GetAnimator();

		switch (_currentState)
		{
		case PlayerAnimState::Idle:
			if (_playerMovement->IsFacingLeft())
				animator.Play(L"IdleLeft", playOption);
			else
				animator.Play(L"IdleRight", playOption);
			break;

		case PlayerAnimState::Move:
			if (_playerMovement->IsFacingLeft())
				animator.Play(L"MoveLeft", playOption);
			else
				animator.Play(L"MoveRight", playOption);
			break;
		}
	}

	void PlayerAnimationFSMComponent::OnAnimationNotify(const std::wstring& notifyName)
	{
#ifdef _DEBUG
		if (notifyName == L"MoveLeftStep")
			OutputDebugStringW(L"[PlayerAnimationFSMComponent] 문자열 기반 AnimationNotify: MoveLeftStep\n");
		else if (notifyName == L"MoveRightStep")
			OutputDebugStringW(L"[PlayerAnimationFSMComponent] 문자열 기반 AnimationNotify: MoveRightStep\n");
#endif
	}
}
