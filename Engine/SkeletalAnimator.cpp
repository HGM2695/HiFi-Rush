#include "SkeletalAnimator.h"
#include "AnimationClipSet.h"
#include "AnimationController.h"
#include "AnimationNotify.h"
#include "Application.h"
#include "Resources.h"
#include "SkeletalAnimationClip.h"
#include "SkeletalMesh.h"
#include "SkeletalPose.h"

namespace gm
{
	SkeletalAnimator::SkeletalAnimator()
		: _animationClipSet(std::make_unique<AnimationClipSet>())
		, _animationController(std::make_unique<AnimationController>())
		, _animationNotifyDispatcher(std::make_unique<AnimationNotifyDispatcher>())
	{
	}

	SkeletalAnimator::~SkeletalAnimator() = default;

	void SkeletalAnimator::Tick(float deltaTime, const SkeletalMesh& skeletalMesh, SkeletalPose& pose)
	{
		if (_currentClip == nullptr)
			return;

		if (_animationController->IsPlaying())
		{
			_animationController->Tick(deltaTime);
			_animationNotifyDispatcher->Dispatch(_currentClip->GetNotifyEvents(), _animationController->GetPlayTime(), _currentClip->GetLength());
		}

		pose.ApplyAnimation(skeletalMesh, *_currentClip, _animationController->GetPlayTime());
	}

	bool SkeletalAnimator::AddClip(const std::wstring& name, const std::wstring& clipKey)
	{
		return _animationClipSet->AddClip(name, APPLICATION.GetResources().Find<SkeletalAnimationClip>(clipKey));
	}

	bool SkeletalAnimator::AddClip(const std::wstring& name, const std::shared_ptr<SkeletalAnimationClip>& clip)
	{
		return _animationClipSet->AddClip(name, clip);
	}

	std::shared_ptr<SkeletalAnimationClip> SkeletalAnimator::FindClip(const std::wstring& name) const
	{
		return _animationClipSet->FindClip<SkeletalAnimationClip>(name);
	}

	NotifyConnection SkeletalAnimator::BindNotifyListener(const AnimationNotifyListener& notifyListener)
	{
		return _animationNotifyDispatcher->BindNotifyListener(notifyListener);
	}

	void SkeletalAnimator::ClearNotifyListeners()
	{
		_animationNotifyDispatcher->ClearNotifyListeners();
	}

	bool SkeletalAnimator::HasClip(const std::wstring& name) const
	{
		return _animationClipSet->HasClip(name);
	}

	bool SkeletalAnimator::Play(const std::wstring& name, const AnimationPlayOption& option)
	{
		std::shared_ptr<SkeletalAnimationClip> clip = FindClip(name);
		GM_ASSERT_RETURN_VAL(clip, false, "요청한 이름의 SkeletalAnimationClip이 없습니다.");

		_currentClip = clip;
		const bool played = _animationController->Play(*_currentClip, option);
		_animationNotifyDispatcher->Reset(_animationController->GetPlayTime());
		return played;
	}

	void SkeletalAnimator::Reset()
	{
		_animationController->Reset();
		_animationNotifyDispatcher->Reset();
	}

	void SkeletalAnimator::Pause()
	{
		_animationController->Pause();
	}

	void SkeletalAnimator::Resume()
	{
		_animationController->Resume();
	}

	void SkeletalAnimator::SetPlayRate(float playRate)
	{
		_animationController->SetPlayRate(playRate);
	}

	AnimationState SkeletalAnimator::GetState() const
	{
		return _animationController->GetState();
	}

	float SkeletalAnimator::GetPlayTime() const
	{
		return _animationController->GetPlayTime();
	}

	float SkeletalAnimator::GetPlayRate() const
	{
		return _animationController->GetPlayRate();
	}

	bool SkeletalAnimator::IsLoop() const
	{
		return _animationController->IsLoop();
	}
}
