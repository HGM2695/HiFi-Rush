#pragma once

#include "AnimationNotify.h"
#include "AnimationTypes.h"
#include "Component.h"

namespace gm
{
	class AnimatedSpriteComponent;
	class PlayerMovementComponent;

	enum class PlayerAnimState
	{
		None,
		Idle,
		Move,
	};

	class PlayerAnimationFSMComponent : public Component
	{
	public:
		virtual TickGroup GetTickGroup() const override { return TickGroup::PostPhysics; }

	protected:
		virtual void	OnInitialize() override;
		virtual void	OnTick(float deltaTime) override;

	private:
		void			OnAnimationNotify(const std::wstring& notifyName);
		void			SyncDirection();
		void			UpdateState();
		void			ChangeState(PlayerAnimState nextState);
		void			PlayCurrentAnimation(const AnimationPlayOption& playOption = {});

	private:
		PlayerAnimState				_currentState = PlayerAnimState::None;
		Vector2						_lastFacingDirection{ 1.f, 0.f };

		NotifyConnection			_notifyConnection{};
		AnimatedSpriteComponent*	_animatedSpriteComponent = nullptr;
		PlayerMovementComponent*				_playerMovement = nullptr;
	};
}
