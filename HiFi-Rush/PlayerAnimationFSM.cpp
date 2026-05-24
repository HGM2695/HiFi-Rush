#include "PlayerAnimationFSM.h"
#include "AnimatedSpriteComponent.h"
#include "GameObject.h"
#include "PlayerMovement.h"
#include "SpriteAnimator.h"
#include <windows.h>

namespace gm
{
	void PlayerAnimationFSM::OnInitialize()
	{
		_animatedSpriteComponent = GetOwner().GetComponent<AnimatedSpriteComponent>();
		GM_ASSERT(_animatedSpriteComponent, "PlayerAnimationFSM은 AnimatedSpriteComponent가 필요합니다.");

		SpriteAnimator& animator = _animatedSpriteComponent->GetAnimator();
		_notifyConnection = animator.BindNotifyListener(
			[this](const std::wstring& notifyName)
			{
				OnAnimationNotify(notifyName);
			});

		_playerMovement = GetOwner().GetComponent<PlayerMovement>();
		GM_ASSERT(_playerMovement, "PlayerAnimationFSM은 PlayerMovement가 필요합니다.");
		_lastFacingDirection = _playerMovement->GetFacingDirection();

		ChangeState(PlayerAnimState::Idle);
	}

	void PlayerAnimationFSM::OnTick(float deltaTime)
	{
		SyncDirection();
		UpdateState();
	}

	void PlayerAnimationFSM::SyncDirection()
	{
		const Vector2 facingDirection = _playerMovement->GetFacingDirection();
		if (_lastFacingDirection == facingDirection)
			return;

		_lastFacingDirection = facingDirection;
		PlayCurrentAnimation({ _animatedSpriteComponent->GetAnimator().GetPlayTime() });
	}

	void PlayerAnimationFSM::UpdateState()
	{
		if (_playerMovement->IsMoving())
			ChangeState(PlayerAnimState::Move);
		else
			ChangeState(PlayerAnimState::Idle);
	}

	void PlayerAnimationFSM::ChangeState(PlayerAnimState nextState)
	{
		if (_currentState == nextState)
			return;

		_currentState = nextState;
		PlayCurrentAnimation();
	}

	void PlayerAnimationFSM::PlayCurrentAnimation(const AnimationPlayOption& playOption)
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

	void PlayerAnimationFSM::OnAnimationNotify(const std::wstring& notifyName)
	{
#ifdef _DEBUG
		if (notifyName == L"MoveLeftStep")
			OutputDebugStringW(L"[PlayerAnimationFSM] 문자열 기반 AnimationNotify: MoveLeftStep\n");
		else if (notifyName == L"MoveRightStep")
			OutputDebugStringW(L"[PlayerAnimationFSM] 문자열 기반 AnimationNotify: MoveRightStep\n");
#endif
	}
}
