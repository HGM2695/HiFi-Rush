#include "BeatVisibilityComponent.h"
#include "BeatSystem.h"
#include "GameObject.h"

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

	void BeatVisibilityComponent::Reset()
	{
		GetOwner().SetRender(_desc.initialVisible);
		_triggerBeat = 0.f;
		_state = VisibilityState::Initial;
	}

	void BeatVisibilityComponent::OnInitialize()
	{
		Reset();
	}

	void BeatVisibilityComponent::OnTick(float)
	{
		if (_state != VisibilityState::Scheduled || _beatSystem.HasPlaybackTime() == false || _beatSystem.GetCurrentBeat() < _triggerBeat)
			return;

		GetOwner().SetRender(_desc.visibleOnTrigger);
		_state = VisibilityState::Triggered;
	}
}
