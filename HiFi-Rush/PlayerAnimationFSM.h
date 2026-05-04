#pragma once

#include "AnimationNotify.h"
#include "Component.h"
#include "AnimationTypes.h"
#include "Transform.h"

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
		bool			IsLookingLeft(const Vector2& forward) const { return forward.x < 0.f; }
		void			SyncDirection();
		void			UpdateState();
		void			ChangeState(PlayerAnimState nextState);
		void			PlayCurrentAnimation(const AnimationPlayOption& playOption = {});

	private:
		PlayerAnimState		_currentState = PlayerAnimState::None;
		Vector2				_lastForward{ 1.f, 0.f };

		NotifyConnection	_notifyConnection{};
		SpriteAnimator*		_spriteAnimator = nullptr;
		PlayerMovement*		_playerMovement = nullptr;
		Transform*			_ownerTransform = nullptr;
	};
}
