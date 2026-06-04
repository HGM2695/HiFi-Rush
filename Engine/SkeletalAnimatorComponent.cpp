#include "SkeletalAnimatorComponent.h"
#include "AnimationClipSet.h"
#include "AnimationController.h"
#include "AnimationNotify.h"
#include "Application.h"
#include "GameObject.h"
#include "Resources.h"
#include "SkeletalAnimationClip.h"
#include "SkeletalMesh.h"
#include "SkeletalMeshComponent.h"
#include "SkeletalPose.h"

namespace gm
{
	SkeletalAnimatorComponent::SkeletalAnimatorComponent()
		: _animationClipSet(std::make_unique<AnimationClipSet>())
		, _animationController(std::make_unique<AnimationController>())
		, _animationNotifyDispatcher(std::make_unique<AnimationNotifyDispatcher>())
	{}

	SkeletalAnimatorComponent::~SkeletalAnimatorComponent() = default;

	void SkeletalAnimatorComponent::OnInitialize()
	{
		_skeletalMeshComponent = GetOwner().GetComponent<SkeletalMeshComponent>();
		GM_ASSERT_RETURN(_skeletalMeshComponent, "SkeletalAnimatorComponent는 SkeletalMeshComponent를 요구합니다.");

		const std::shared_ptr<SkeletalMesh>& skeletalMesh = _skeletalMeshComponent->GetSkeletalMesh();
		GM_ASSERT_RETURN(skeletalMesh, "SkeletalMeshComponent에 SkeletalMesh가 존재하지 않습니다.");
		ResolveRootMotionBoneIndex(*skeletalMesh);
	}

	void SkeletalAnimatorComponent::OnTick(float deltaTime)
	{
		if (_skeletalMeshComponent == nullptr || _currentClip == nullptr)
			return;

		const std::shared_ptr<SkeletalMesh>& skeletalMesh = _skeletalMeshComponent->GetSkeletalMesh();
		if (skeletalMesh == nullptr)
			return;

		SkeletalPose& pose = _skeletalMeshComponent->GetPose();
		if (_animationController->IsPlaying())
		{
			_animationController->Tick(deltaTime);
			_animationNotifyDispatcher->Dispatch(_currentClip->GetNotifyEvents(), _animationController->GetPlayTime(), _currentClip->GetLength());
		}

		const SkeletalPoseApplyResult result = pose.ApplyAnimation(*skeletalMesh, *_currentClip, _animationController->GetPlayTime(), _rootMotionBoneIndex);
		UpdateRootMotion(result);
		_animationBlender.Tick(deltaTime, pose);
	}

	bool SkeletalAnimatorComponent::Play(const std::wstring& name, const AnimationPlayOption& option)
	{
		GM_ASSERT_RETURN_VAL(_skeletalMeshComponent, false, "SkeletalAnimatorComponent가 SkeletalMeshComponent와 연결되지 않았습니다.");

		std::shared_ptr<SkeletalAnimationClip> clip = FindClip(name);
		GM_ASSERT_RETURN_VAL(clip, false, "요청한 이름의 SkeletalAnimationClip이 없습니다.");

		if (_currentClip && option.blendDuration > 0.f)
			_animationBlender.BeginBlend(_skeletalMeshComponent->GetPose(), option.blendDuration);
		else
			_animationBlender.Reset();

		_currentClip = clip;
		const bool played = _animationController->Play(*_currentClip, option);
		_animationNotifyDispatcher->Reset(_animationController->GetPlayTime());
		ResetRootMotion();
		return played;
	}

	bool SkeletalAnimatorComponent::AddClip(const std::wstring& name, const std::wstring& clipKey)
	{
		return _animationClipSet->AddClip(name, APPLICATION.GetResources().Find<SkeletalAnimationClip>(clipKey));
	}

	bool SkeletalAnimatorComponent::AddClip(const std::wstring& name, const std::shared_ptr<SkeletalAnimationClip>& clip)
	{
		return _animationClipSet->AddClip(name, clip);
	}

	std::shared_ptr<SkeletalAnimationClip> SkeletalAnimatorComponent::FindClip(const std::wstring& name) const
	{
		return _animationClipSet->FindClip<SkeletalAnimationClip>(name);
	}

	NotifyConnection SkeletalAnimatorComponent::BindNotifyListener(const AnimationNotifyListener& notifyListener)
	{
		return _animationNotifyDispatcher->BindNotifyListener(notifyListener);
	}

	void SkeletalAnimatorComponent::ClearNotifyListeners()
	{
		_animationNotifyDispatcher->ClearNotifyListeners();
	}

	bool SkeletalAnimatorComponent::HasClip(const std::wstring& name) const
	{
		return _animationClipSet->HasClip(name);
	}

	void SkeletalAnimatorComponent::Reset()
	{
		_animationController->Reset();
		_animationNotifyDispatcher->Reset();
		_animationBlender.Reset();
	}

	void SkeletalAnimatorComponent::Pause()
	{
		_animationController->Pause();
	}

	void SkeletalAnimatorComponent::Resume()
	{
		_animationController->Resume();
	}

	void SkeletalAnimatorComponent::SetPlayRate(float playRate)
	{
		_animationController->SetPlayRate(playRate);
	}

	void SkeletalAnimatorComponent::SetRootMotionBoneName(const std::wstring& boneName)
	{
		_rootMotionBoneName = boneName;
		_rootMotionBoneIndex = -1;
		ResetRootMotion();
	}

	Vector3 SkeletalAnimatorComponent::ConsumeRootMotionDelta()
	{
		const Vector3 delta = _rootMotionDelta;
		_rootMotionDelta = Vector3{};
		return delta;
	}

	void SkeletalAnimatorComponent::ResolveRootMotionBoneIndex(const SkeletalMesh& skeletalMesh)
	{
		if (_rootMotionBoneIndex >= 0 || _rootMotionBoneName.empty())
			return;

		const std::vector<BoneData>& bones = skeletalMesh.GetBones();
		for (uint32 boneIndex = 0; boneIndex < bones.size(); ++boneIndex)
		{
			if (bones[boneIndex].name == _rootMotionBoneName)
			{
				_rootMotionBoneIndex = static_cast<int32>(boneIndex);
				return;
			}
		}
	}

	void SkeletalAnimatorComponent::UpdateRootMotion(const SkeletalPoseApplyResult& result)
	{
		if (result.hasRootMotion == false)
		{
			_hasPreviousRootMotionPosition = false;
			return;
		}

		if (_hasPreviousRootMotionPosition)
			_rootMotionDelta += (result.rootMotionPosition - _previousRootMotionPosition) * _rootMotionScale;

		_previousRootMotionPosition = result.rootMotionPosition;
		_hasPreviousRootMotionPosition = true;
	}

	void SkeletalAnimatorComponent::ResetRootMotion()
	{
		_previousRootMotionPosition = Vector3{};
		_rootMotionDelta = Vector3{};
		_hasPreviousRootMotionPosition = false;
	}

	AnimationState SkeletalAnimatorComponent::GetState() const
	{
		return _animationController->GetState();
	}

	float SkeletalAnimatorComponent::GetPlayTime() const
	{
		return _animationController->GetPlayTime();
	}

	float SkeletalAnimatorComponent::GetPlayRate() const
	{
		return _animationController->GetPlayRate();
	}

	bool SkeletalAnimatorComponent::IsLoop() const
	{
		return _animationController->IsLoop();
	}
}
