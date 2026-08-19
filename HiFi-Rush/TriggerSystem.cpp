#include "TriggerSystem.h"
#include "BeatSystem.h"

#if GM_ENABLE_DEBUG_TOOLS
#include "DebugEventPublisher.h"
#include "HiFiRushStatics.h"
#endif

namespace gm
{
	TriggerSystem::TriggerSystem(const BeatSystem& beatSystem)
		: _beatSystem(beatSystem)
	{
#if GM_ENABLE_DEBUG_TOOLS
		HiFiRushStatics::GetDebugEventPublisher().OnDebugEvent.Subscribe(_debugEventConnection,
			[this](const DebugEvent& event)
			{
				if (event.type == DebugEventType::Activate)
					ActivateAll();
				else if (event.type == DebugEventType::Reset)
					ResetAll();
			});
#endif
	}

	bool TriggerSystem::Activate(const std::wstring& triggerId)
	{
		GM_ASSERT_RETURN_VAL(triggerId.empty() == false, false, "Trigger ID는 비어 있을 수 없습니다.");
		if (_beatSystem.HasPlaybackTime() == false || _activatedTriggerIds.contains(triggerId))
			return true;

		const float triggerStartBeat = _beatSystem.GetNextBeat();
		_activatedTriggerIds.insert(triggerId);
		PublishTrigger(TriggerType::Activate, triggerId, triggerStartBeat, false);
		return true;
	}

	bool TriggerSystem::Reset(const std::wstring& triggerId)
	{
		GM_ASSERT_RETURN_VAL(triggerId.empty() == false, false, "Trigger ID는 비어 있을 수 없습니다.");
		_activatedTriggerIds.erase(triggerId);
		PublishTrigger(TriggerType::Reset, triggerId, 0.f, false);
		return true;
	}

	void TriggerSystem::Clear()
	{
		_activatedTriggerIds.clear();
		OnTrigger.Clear();
	}

	void TriggerSystem::ActivateAll()
	{
		if (_beatSystem.HasPlaybackTime() == false)
			return;

		PublishTrigger(TriggerType::Activate, L"", _beatSystem.GetNextBeat(), true);
	}

	void TriggerSystem::ResetAll()
	{
		_activatedTriggerIds.clear();
		PublishTrigger(TriggerType::Reset, L"", 0.f, true);
	}

	void TriggerSystem::PublishTrigger(TriggerType type, const std::wstring& triggerId, float startBeat, bool targetsAll)
	{
		TriggerEvent event{};
		event.type = type;
		event.triggerId = triggerId;
		event.startBeat = startBeat;
		event.targetsAll = targetsAll;
		OnTrigger.Publish(event);
	}
}
