#include "SpriteAnimator.h"
#include "AnimationController.h"
#include "AnimationClipSet.h"
#include "Application.h"
#include "GMAssert.h"
#include "GameObject.h"
#include "SpriteAnimationClip.h"
#include "SpriteRenderer.h"
#include "Time.h"

namespace gm
{
	SpriteAnimator::SpriteAnimator()
		: _animationController(std::make_unique<AnimationController>())
		, _animationClipSet(std::make_unique<AnimationClipSet>())
	{
	}

	SpriteAnimator::~SpriteAnimator() = default;

	bool SpriteAnimator::AddClip(const std::wstring& name, const std::shared_ptr<SpriteAnimationClip>& clip)
	{
		return _animationClipSet->AddClip(name, clip);
	}

	std::shared_ptr<SpriteAnimationClip> SpriteAnimator::FindClip(const std::wstring& name) const
	{
		// SpriteAnimator는 AddClip()을 통해서 SpriteAnimationClip만 등록한다는 전제.
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
		return played;
	}

	void SpriteAnimator::Reset()
	{
		_animationController->Reset();
		ApplyRenderInfo();
	}

	void SpriteAnimator::Pause()
	{
		_animationController->Pause();
	}

	void SpriteAnimator::Resume()
	{
		_animationController->Resume();
	}

	AnimationState SpriteAnimator::GetState() const
	{
		return _animationController->GetState();
	}

	float SpriteAnimator::GetPlayTime() const
	{
		return _animationController->GetPlayTime();
	}

	bool SpriteAnimator::IsLoop() const
	{
		return _animationController->IsLoop();
	}

	void SpriteAnimator::OnInitialize()
	{
		_spriteRenderer = GetOwner().GetComponent<SpriteRenderer>();
		GM_ASSERT(_spriteRenderer, "SpriteAnimator는 SpriteRenderer가 필요합니다.");

		ApplyRenderInfo();
	}

	void SpriteAnimator::OnUpdate()
	{
		_animationController->Update(APPLICATION.GetTime().GetDeltaTime());
		ApplyRenderInfo();
	}

	void SpriteAnimator::ApplyRenderInfo()
	{
		if (_currentClip == nullptr)
			return;

		GM_ASSERT_RETURN(_spriteRenderer, "OnInitialize()가 먼저 호출되어야 합니다.");

		_spriteRenderer->SetTexture(_currentClip->GetTexture());
		_spriteRenderer->SetSourceRect(_currentClip->GetFrameByTime(GetPlayTime()));
	}
}
