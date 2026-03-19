#include "PlayerAnimationFSM.h"
#include "PlayerMovement.h"
#include <windows.h>
#include "../Engine/GameObject.h"
#include "../Engine/GMAssert.h"
#include "../Engine/SpriteAnimator.h"

namespace gm
{
	void PlayerAnimationFSM::OnInitialize()
	{
		_spriteAnimator = GetOwner().GetComponent<SpriteAnimator>();
		GM_ASSERT(_spriteAnimator, "PlayerAnimationFSM은 SpriteAnimator가 필요합니다.");
		_notifyConnection = _spriteAnimator->BindNotifyCallback(
			[this](const std::wstring& notifyName)
			{
				OnAnimationNotify(notifyName);
			});

		_playerMovement = GetOwner().GetComponent<PlayerMovement>();
		GM_ASSERT(_playerMovement, "PlayerAnimationFSM은 PlayerMovement가 필요합니다.");

		_ownerTransform = GetOwner().GetTransform();
		GM_ASSERT(_ownerTransform, "PlayerAnimationFSM은 Transform이 필요합니다.");
		_lastForward = _ownerTransform->GetForward();

		ChangeState(PlayerAnimState::Idle);
	}

	void PlayerAnimationFSM::OnLateUpdate()
	{
		SyncDirection();
		UpdateState();
	}

	void PlayerAnimationFSM::SyncDirection()
	{
		const math::Vector2 forward = _ownerTransform->GetForward();
		if (_lastForward == forward)
			return;

		_lastForward = forward;
		PlayCurrentAnimation({ _spriteAnimator->GetPlayTime() });
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
		switch (_currentState)
		{
		case PlayerAnimState::Idle:
			if (IsLookingLeft(_lastForward))
				_spriteAnimator->Play(L"IdleLeft", playOption);
			else
				_spriteAnimator->Play(L"IdleRight", playOption);
			break;

		case PlayerAnimState::Move:
			if (IsLookingLeft(_lastForward))
				_spriteAnimator->Play(L"MoveLeft", playOption);
			else
				_spriteAnimator->Play(L"MoveRight", playOption);
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
