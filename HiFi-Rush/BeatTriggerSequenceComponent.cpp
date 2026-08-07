#include "BeatTriggerSequenceComponent.h"
#include "BeatSystem.h"
#include "IBeatTriggerAction.h"

#if GM_ENABLE_DEBUG_TOOLS
#include "DebugEventPublisher.h"
#include "HiFiRushStatics.h"
#endif

#include <cmath>

namespace gm
{
	BeatTriggerSequenceComponent::BeatTriggerSequenceComponent(const BeatSystem& beatSystem)
		: _beatSystem(beatSystem)
	{}

	void BeatTriggerSequenceComponent::AddAction(float beatOffset, const WeakGameObjectPtr& actionOwner, IBeatTriggerAction& action)
	{
		GM_ASSERT_RETURN(beatOffset >= 0.f, "BeatTriggerSequenceComponent의 beatOffset은 0 이상이어야 합니다.");
		GM_ASSERT_RETURN(actionOwner.IsValid(), "BeatTriggerSequenceComponent에 유효하지 않은 Action Owner를 추가할 수 없습니다.");
		GM_ASSERT_RETURN(_isActivated == false, "활성화된 BeatTriggerSequenceComponent에는 Action을 추가할 수 없습니다.");

		_entries.push_back(SequenceEntry{ actionOwner, &action, beatOffset });
	}

	void BeatTriggerSequenceComponent::Activate()
	{
		if (_isActivated || _beatSystem.HasPlaybackTime() == false)
			return;

		const float sequenceStartBeat = std::floor(_beatSystem.GetCurrentBeat()) + 1.f;
		for (const SequenceEntry& entry : _entries)
		{
			if (entry.actionOwner.IsValid() == false)
				continue;

			entry.action->Schedule(sequenceStartBeat + entry.beatOffset);
		}

		_isActivated = true;
	}

	void BeatTriggerSequenceComponent::Reset()
	{
		for (const SequenceEntry& entry : _entries)
		{
			if (entry.actionOwner.IsValid() == false)
				continue;

			entry.action->Reset();
		}

		_isActivated = false;
	}

	void BeatTriggerSequenceComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(_entries.empty() == false, "BeatTriggerSequenceComponent에는 하나 이상의 Action이 필요합니다.");

#if GM_ENABLE_DEBUG_TOOLS
		HiFiRushStatics::GetDebugEventPublisher().OnDebugEvent.Subscribe(_debugEventConnection,
			[this](const DebugEvent& event)
			{
				if (event.type == DebugEventType::Activate)
					Activate();
				else if (event.type == DebugEventType::Reset)
					Reset();
			});
#endif
	}
}
