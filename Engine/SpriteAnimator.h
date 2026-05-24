#pragma once

#include "AnimationNotify.h"
#include "AnimationTypes.h"
#include "IAnimator.h"
#include <memory>

namespace gm
{
	class AnimationController;
	class AnimationClipSet;
	class AnimationNotifyDispatcher;
	class SpriteAnimationClip;
	class SpritePresenter;

	class SpriteAnimator : public IAnimator
	{
	public:
		SpriteAnimator();
		virtual ~SpriteAnimator();

		bool									AddClip(const std::wstring& name, const std::wstring& clipKey);
		bool									AddClip(const std::wstring& name, const std::shared_ptr<SpriteAnimationClip>& clip);
		std::shared_ptr<SpriteAnimationClip>		FindClip(const std::wstring& name) const;
		std::shared_ptr<SpriteAnimationClip>		GetCurrentClip() const { return _currentClip; }

		NotifyConnection						BindNotifyListener(const AnimationNotifyListener& notifyListener);
		void									ClearNotifyListeners();
		void									Tick(float deltaTime, SpritePresenter& presenter);

		bool									HasClip(const std::wstring& name) const override;
		bool									Play(const std::wstring& name, const AnimationPlayOption& option = {}) override;
		void									Reset() override;
		void									Pause() override;
		void									Resume() override;
		AnimationState							GetState() const override;
		float									GetPlayTime() const override;
		bool									IsLoop() const override;

	private:
		void									UpdateRenderInfo(SpritePresenter& presenter);

	private:
		std::unique_ptr<AnimationClipSet>			_animationClipSet;
		std::shared_ptr<SpriteAnimationClip>			_currentClip{};
		std::unique_ptr<AnimationController>			_animationController;
		std::unique_ptr<AnimationNotifyDispatcher>	_animationNotifyDispatcher;
	};
}
