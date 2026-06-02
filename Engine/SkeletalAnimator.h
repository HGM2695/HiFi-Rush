#pragma once

#include "AnimationNotify.h"
#include "AnimationTypes.h"

#include <memory>

namespace gm
{
	class AnimationClipSet;
	class AnimationController;
	class AnimationNotifyDispatcher;
	class SkeletalAnimationClip;
	class SkeletalMesh;
	class SkeletalPose;

	class SkeletalAnimator
	{
	public:
		SkeletalAnimator();
		~SkeletalAnimator();

		bool									AddClip(const std::wstring& name, const std::wstring& clipKey);
		bool									AddClip(const std::wstring& name, const std::shared_ptr<SkeletalAnimationClip>& clip);
		std::shared_ptr<SkeletalAnimationClip>	FindClip(const std::wstring& name) const;
		std::shared_ptr<SkeletalAnimationClip>	GetCurrentClip() const { return _currentClip; }

		NotifyConnection						BindNotifyListener(const AnimationNotifyListener& notifyListener);
		void									ClearNotifyListeners();
		void									Tick(float deltaTime, const SkeletalMesh& skeletalMesh, SkeletalPose& pose);

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
		std::unique_ptr<AnimationClipSet>			_animationClipSet;
		std::shared_ptr<SkeletalAnimationClip>		_currentClip;
		std::unique_ptr<AnimationController>		_animationController;
		std::unique_ptr<AnimationNotifyDispatcher>	_animationNotifyDispatcher;
	};
}
