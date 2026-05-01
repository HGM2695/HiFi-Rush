#pragma once

#include "AnimationNotify.h"
#include "Component.h"
#include "IAnimator.h"
#include <memory>
#include <string>

namespace gm
{
	class AnimationController;
	class AnimationClipSet;
	class AnimationNotifyDispatcher;
	class SpriteAnimationClip;
	class SpriteRenderer;

	class SpriteAnimator : public Component, public IAnimator
	{
	public:
		SpriteAnimator();
		virtual ~SpriteAnimator();

		bool									AddClip(const std::wstring& name, const std::wstring& clipKey);
		bool									AddClip(const std::wstring& name, const std::shared_ptr<SpriteAnimationClip>& clip);
		std::shared_ptr<SpriteAnimationClip>	FindClip(const std::wstring& name) const;
		std::shared_ptr<SpriteAnimationClip>	GetCurrentClip() const { return _currentClip; }

		[[nodiscard]] 
		NotifyConnection						BindNotifyCallback(const AnimationNotifyCallback& notifyCallback);
		void									ClearNotifyCallbacks();

		// IAnimator
		bool									HasClip(const std::wstring& name) const override;
		bool									Play(const std::wstring& name, const AnimationPlayOption& option = {}) override;
		virtual void							Reset() override;
		virtual void							Pause() override;
		virtual void							Resume() override;
		virtual AnimationState					GetState() const override;
		virtual float							GetPlayTime() const override;
		virtual bool							IsLoop() const override;

	protected:
		virtual void							OnInitialize() override;
		virtual void							OnUpdate() override;

	private:
		void									ApplyRenderInfo();

	private:		
		std::unique_ptr<AnimationClipSet>			_animationClipSet;
		std::shared_ptr<SpriteAnimationClip>		_currentClip{};
		std::unique_ptr<AnimationController>		_animationController;
		std::unique_ptr<AnimationNotifyDispatcher>	_animationNotifyDispatcher;
		SpriteRenderer*								_spriteRenderer = nullptr;
	};
}
