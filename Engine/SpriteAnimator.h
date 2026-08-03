#pragma once

#include "AnimationNotify.h"
#include "AnimationTypes.h"
#include <memory>

namespace gm
	{
		class AnimationController;
		class AnimationClipSet;
		class SpriteAnimationClip;
	class SpritePresenter;

	class SpriteAnimator
	{
	public:
		SpriteAnimator();
		~SpriteAnimator();

		bool									AddClip(const std::wstring& name, const std::wstring& clipKey);
		bool									AddClip(const std::wstring& name, const std::shared_ptr<SpriteAnimationClip>& clip);
		std::shared_ptr<SpriteAnimationClip>	FindClip(const std::wstring& name) const;
		std::shared_ptr<SpriteAnimationClip>	GetCurrentClip() const { return _currentClip; }
		EventPublisher<AnimationNotifyDispatcher, AnimationNotifyEvent>& GetNotifyEvent() { return _animationNotifyDispatcher->OnNotify; }

		void									Tick(float deltaTime, SpritePresenter& presenter);

		bool									HasClip(const std::wstring& name) const;
		bool									Play(const std::wstring& name, const AnimationPlayOption& option = {});
		void									Reset();
		void									Pause();
		void									Resume();
		void									SetPlayRate(float playRate);
		AnimationState							GetState() const;
		float									GetPlayTime() const;
		float									GetPlayRate() const;
		bool									IsLoop() const;

	private:
		void									UpdateRenderInfo(SpritePresenter& presenter);

	private:
		std::unique_ptr<AnimationClipSet>			_animationClipSet;
		std::shared_ptr<SpriteAnimationClip>		_currentClip{};
		std::unique_ptr<AnimationController>		_animationController;
		std::unique_ptr<AnimationNotifyDispatcher>	_animationNotifyDispatcher;
	};
}
