#pragma once

#include "AnimationNotify.h"
#include "Component.h"
#include "AnimationTypes.h"

namespace gm
{
	class PlayerMovement;
	class SpriteAnimator;

	enum class PlayerAnimState
	{
		None,
		Idle,
		Move,
	};

	class PlayerAnimationFSM : public Component
	{
	protected:
		virtual void	OnInitialize() override;
		virtual void	OnLateUpdate() override;

	private:
		void			OnAnimationNotify(const std::wstring& notifyName);
		void			SyncDirection();
		void			UpdateState();
		void			ChangeState(PlayerAnimState nextState);
		void			PlayCurrentAnimation(const AnimationPlayOption& playOption = {});

	private:
		PlayerAnimState		_currentState = PlayerAnimState::None;
		Vector2				_lastFacingDirection{ 1.f, 0.f };

		NotifyConnection	_notifyConnection{};
		SpriteAnimator*		_spriteAnimator = nullptr;
		PlayerMovement*		_playerMovement = nullptr;
	};
}
