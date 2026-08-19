#include "BeatVisibilityComponent.h"
#include "BeatSystem.h"
#include "GameObject.h"
#include "GameplayScene.h"

namespace gm
{
	BeatVisibilityComponent::BeatVisibilityComponent(const BeatSystem& beatSystem, const BeatVisibilityDesc& desc)
		: _beatSystem(beatSystem), _desc(desc)
	{}

	void BeatVisibilityComponent::Schedule(float startBeat)
	{
		if (_state != VisibilityState::Initial)
			return;

		_triggerBeat = startBeat;
		_state = VisibilityState::Scheduled;
	}

	void BeatVisibilityComponent::ResetAction()
	{
		GetOwner().SetRender(_desc.initialVisible);
		_triggerBeat = 0.f;
		_state = VisibilityState::Initial;
	}

	void BeatVisibilityComponent::OnInitialize()
	{
		ResetAction();

		GameplayScene* scene = dynamic_cast<GameplayScene*>(GetOwner().GetScene());
		GM_ASSERT_RETURN(scene, "BeatVisibilityComponent는 GameplayScene에서만 사용할 수 있습니다.");
		GM_ASSERT_RETURN(_triggerBinding.Bind(scene->GetTriggerSystem(), _desc.triggerId, _desc.beatOffset,
			[this](float startBeat) { Schedule(startBeat); },
			[this]() { ResetAction(); }), "BeatVisibilityComponent의 Trigger Binding에 실패했습니다.");
	}

	void BeatVisibilityComponent::OnTick(float)
	{
		if (_state != VisibilityState::Scheduled || _beatSystem.HasPlaybackTime() == false || _beatSystem.GetCurrentBeat() < _triggerBeat)
			return;

		GetOwner().SetRender(_desc.visibleOnTrigger);
		_state = VisibilityState::Triggered;
	}
}
