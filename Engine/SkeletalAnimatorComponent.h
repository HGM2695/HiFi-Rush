#pragma once

#include "AnimationNotify.h"
#include "AnimationTypes.h"
#include "Component.h"
#include "SkeletalAnimationBlender.h"

#include <memory>

namespace gm
{
	class AnimationClipSet;
	class AnimationController;
	class AnimationNotifyDispatcher;
	class SkeletalAnimationClip;
	class SkeletalMesh;
	class SkeletalMeshComponent;
	struct SkeletalPoseApplyResult;

	class SkeletalAnimatorComponent : public Component
	{
	public:
		SkeletalAnimatorComponent();
		virtual ~SkeletalAnimatorComponent();

		virtual TickGroup GetTickGroup() const override { return TickGroup::Animation; }

		bool									AddClip(const std::wstring& name, const std::wstring& clipKey);
		bool									AddClip(const std::wstring& name, const std::shared_ptr<SkeletalAnimationClip>& clip);
		std::shared_ptr<SkeletalAnimationClip>	FindClip(const std::wstring& name) const;
		std::shared_ptr<SkeletalAnimationClip>	GetCurrentClip() const { return _currentClip; }

		NotifyConnection						BindNotifyListener(const AnimationNotifyListener& notifyListener);
		void									ClearNotifyListeners();

		bool									HasClip(const std::wstring& name) const;
		bool									Play(const std::wstring& name, const AnimationPlayOption& option = {});
		void									Reset();
		void									Pause();
		void									Resume();
		void									SetPlayRate(float playRate);
		void									SetRootMotionBoneName(const std::wstring& boneName);
		void									SetRootMotionScale(float scale) { _rootMotionScale = scale; }
		Vector3									ConsumeRootMotionDelta();
		AnimationState							GetState() const;
		float									GetPlayTime() const;
		float									GetPlayRate() const;
		bool									IsLoop() const;

	protected:
		virtual void							OnInitialize() override;
		virtual void							OnTick(float deltaTime) override;

	private:
		void									ResolveRootMotionBoneIndex(const SkeletalMesh& skeletalMesh);
		void									UpdateRootMotion(const SkeletalPoseApplyResult& result);
		void									ResetRootMotion();

	private:
		SkeletalMeshComponent*						_skeletalMeshComponent = nullptr;

		std::unique_ptr<AnimationClipSet>			_animationClipSet;
		std::shared_ptr<SkeletalAnimationClip>		_currentClip;
		std::unique_ptr<AnimationController>		_animationController;
		std::unique_ptr<AnimationNotifyDispatcher>	_animationNotifyDispatcher;
		SkeletalAnimationBlender					_animationBlender;
		std::wstring								_rootMotionBoneName = L"origin_$AssimpFbx$_Translation";
		int32										_rootMotionBoneIndex = -1;
		Vector3										_previousRootMotionPosition{};
		Vector3										_rootMotionDelta{};
		float										_rootMotionScale = 0.01f;
		bool										_hasPreviousRootMotionPosition = false;
	};
}
