#include "SpriteAnimator.h"
#include "AnimationController.h"
#include "AnimationClipSet.h"
#include "AnimationNotify.h"
#include "Application.h"
#include "Resources.h"
#include "SpriteAnimationClip.h"
#include "SpritePresenter.h"

namespace gm
{
	SpriteAnimator::SpriteAnimator()
		: _animationController(std::make_unique<AnimationController>())
		, _animationClipSet(std::make_unique<AnimationClipSet>())
		, _animationNotifyDispatcher(std::make_unique<AnimationNotifyDispatcher>()) {}

	SpriteAnimator::~SpriteAnimator() = default;

	NotifyConnection SpriteAnimator::BindNotifyListener(const AnimationNotifyListener& notifyListener)
	{
		return _animationNotifyDispatcher->BindNotifyListener(notifyListener);
	}

	void SpriteAnimator::ClearNotifyListeners()
	{
		_animationNotifyDispatcher->ClearNotifyListeners();
	}

	bool SpriteAnimator::AddClip(const std::wstring& name, const std::wstring& clipKey)
	{
		return _animationClipSet->AddClip(name, APPLICATION.GetResources().Find<SpriteAnimationClip>(clipKey));
	}

	bool SpriteAnimator::AddClip(const std::wstring& name, const std::shared_ptr<SpriteAnimationClip>& clip)
	{
		return _animationClipSet->AddClip(name, clip);
	}

	std::shared_ptr<SpriteAnimationClip> SpriteAnimator::FindClip(const std::wstring& name) const
	{
		return _animationClipSet->FindClip<SpriteAnimationClip>(name);
	}

	bool SpriteAnimator::HasClip(const std::wstring& name) const
	{
		return _animationClipSet->HasClip(name);
	}

	bool SpriteAnimator::Play(const std::wstring& name, const AnimationPlayOption& option)
	{
		auto clip = FindClip(name);
		GM_ASSERT_RETURN_VAL(clip, false, "요청한 이름의 SpriteAnimationClip이 없습니다.");

		_currentClip = clip;
		const bool played = _animationController->Play(*_currentClip, option);
		_animationNotifyDispatcher->Reset(_animationController->GetPlayTime());
		return played;
	}

	void SpriteAnimator::Reset()
	{
		_animationController->Reset();
		_animationNotifyDispatcher->Reset();
	}

	void SpriteAnimator::Pause()
	{
		_animationController->Pause();
	}

	void SpriteAnimator::Resume()
	{
		_animationController->Resume();
	}

	void SpriteAnimator::SetPlayRate(float playRate)
	{
		_animationController->SetPlayRate(playRate);
	}

	AnimationState SpriteAnimator::GetState() const
	{
		return _animationController->GetState();
	}

	float SpriteAnimator::GetPlayTime() const
	{
		return _animationController->GetPlayTime();
	}

	float SpriteAnimator::GetPlayRate() const
	{
		return _animationController->GetPlayRate();
	}

	bool SpriteAnimator::IsLoop() const
	{
		return _animationController->IsLoop();
	}

	void SpriteAnimator::Tick(float deltaTime, SpritePresenter& presenter)
	{
		if (_currentClip == nullptr)
			return;

		if (_animationController->IsPlaying())
		{
			_animationController->Tick(deltaTime);
			_animationNotifyDispatcher->Dispatch(_currentClip->GetNotifyEvents(), _animationController->GetPlayTime(), _currentClip->GetLength());
		}

		UpdateRenderInfo(presenter);
	}

	void SpriteAnimator::UpdateRenderInfo(SpritePresenter& presenter)
	{
		if (_currentClip == nullptr)
			return;

		presenter.SetTexture(_currentClip->GetTexture(), TextureSlot::BaseColor);
		presenter.SetSourceRect(_currentClip->GetFrameByTime(GetPlayTime()));
	}
}
