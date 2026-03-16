#pragma once

#include "../Engine/Component.h"
#include "../Engine/AnimationTypes.h"
#include "../Engine/Transform.h"

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
		virtual void OnInitialize() override;
		virtual void OnLateUpdate() override;

	private:
		bool IsLookingLeft(const math::Vector2& forward) const { return forward._x < 0.f; }
		void SyncDirection();
		void UpdateState();
		void ChangeState(PlayerAnimState nextState);
		void PlayCurrentAnimation(const AnimationPlayOption& playOption = {});

	private:
		PlayerAnimState	_currentState = PlayerAnimState::None;
		math::Vector2	_lastForward{ 1.f, 0.f };

		SpriteAnimator*	_spriteAnimator = nullptr;
		PlayerMovement*	_playerMovement = nullptr;
		Transform*		_ownerTransform = nullptr;
	};
}
