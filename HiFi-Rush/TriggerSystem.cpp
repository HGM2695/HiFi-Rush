#include "TriggerSystem.h"
#include "BeatSystem.h"

namespace gm
{
	TriggerSystem::TriggerSystem(const BeatSystem& beatSystem)
		: _beatSystem(beatSystem)
	{
	}

	bool TriggerSystem::Activate(const std::wstring& triggerId)
	{
		GM_ASSERT_RETURN_VAL(triggerId.empty() == false, false, "Trigger ID는 비어 있을 수 없습니다.");
		if (_beatSystem.HasPlaybackTime() == false || _activatedTriggerIds.contains(triggerId))
			return true;

		const float triggerStartBeat = _beatSystem.GetNextBeat();
		_activatedTriggerIds.insert(triggerId);
		PublishTrigger(TriggerType::Activate, triggerId, triggerStartBeat);
		return true;
	}

	bool TriggerSystem::Pulse(const std::wstring& triggerId)
	{
		GM_ASSERT_RETURN_VAL(triggerId.empty() == false, false, "Trigger ID는 비어 있을 수 없습니다.");
		if (_beatSystem.HasPlaybackTime() == false)
			return false;

		PublishTrigger(TriggerType::Pulse, triggerId, _beatSystem.GetCurrentBeat());
		return true;
	}

	bool TriggerSystem::Reset(const std::wstring& triggerId)
	{
		GM_ASSERT_RETURN_VAL(triggerId.empty() == false, false, "Trigger ID는 비어 있을 수 없습니다.");
		_activatedTriggerIds.erase(triggerId);
		PublishTrigger(TriggerType::Reset, triggerId, 0.f);
		return true;
	}

	void TriggerSystem::Clear()
	{
		_activatedTriggerIds.clear();
		OnTrigger.Clear();
	}

	void TriggerSystem::PublishTrigger(TriggerType type, const std::wstring& triggerId, float startBeat)
	{
		TriggerEvent event{};
		event.type = type;
		event.triggerId = triggerId;
		event.startBeat = startBeat;
		OnTrigger.Publish(event);
	}
}
