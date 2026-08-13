#include "TriggerSequenceSystem.h"
#include "BeatSystem.h"
#include "IBeatTriggerAction.h"

#if GM_ENABLE_DEBUG_TOOLS
#include "DebugEventPublisher.h"
#include "HiFiRushStatics.h"
#endif

#include <cmath>

namespace gm
{
	TriggerSequenceSystem::TriggerSequenceSystem(const BeatSystem& beatSystem)
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

	bool TriggerSequenceSystem::RegisterAction(const std::wstring& sequenceId, float beatOffset, const WeakGameObjectPtr& actionOwner, IBeatTriggerAction& actionComponent)
	{
		GM_ASSERT_RETURN_VAL(sequenceId.empty() == false, false, "Trigger Sequence ID는 비어 있을 수 없습니다.");
		GM_ASSERT_RETURN_VAL(std::isfinite(beatOffset) && beatOffset >= 0.f, false, "Trigger Sequence의 Beat Offset은 0 이상의 유한한 값이어야 합니다. sequenceId=%ls", sequenceId.c_str());
		GM_ASSERT_RETURN_VAL(actionOwner.IsValid(), false, "Trigger Sequence에 유효하지 않은 Action Owner를 등록할 수 없습니다. sequenceId=%ls", sequenceId.c_str());

		Sequence& sequence = _sequences[sequenceId];
		GM_ASSERT_RETURN_VAL(sequence.isActivated == false, false, "활성화된 Trigger Sequence에는 Action을 등록할 수 없습니다. sequenceId=%ls", sequenceId.c_str());
		sequence.actions.push_back(ActionEntry{ actionOwner, &actionComponent, beatOffset });
		return true;
	}

	bool TriggerSequenceSystem::HasSequence(const std::wstring& sequenceId) const
	{
		return FindSequence(sequenceId) != nullptr;
	}

	bool TriggerSequenceSystem::Activate(const std::wstring& sequenceId)
	{
		Sequence* sequence = FindSequence(sequenceId);
		GM_ASSERT_RETURN_VAL(sequence, false, "Trigger Sequence를 찾을 수 없습니다. sequenceId=%ls", sequenceId.c_str());
		if (sequence->isActivated || _beatSystem.HasPlaybackTime() == false)
			return true;

		const float sequenceStartBeat = std::floor(_beatSystem.GetCurrentBeat()) + 1.f;
		for (const ActionEntry& entry : sequence->actions)
		{
			if (entry.actionOwner.IsValid() == false)
				continue;

			entry.actionComponent->Schedule(sequenceStartBeat + entry.beatOffset);
		}

		sequence->isActivated = true;
		return true;
	}

	bool TriggerSequenceSystem::Reset(const std::wstring& sequenceId)
	{
		Sequence* sequence = FindSequence(sequenceId);
		GM_ASSERT_RETURN_VAL(sequence, false, "Trigger Sequence를 찾을 수 없습니다. sequenceId=%ls", sequenceId.c_str());
		for (const ActionEntry& entry : sequence->actions)
		{
			if (entry.actionOwner.IsValid() == false)
				continue;

			entry.actionComponent->Reset();
		}

		sequence->isActivated = false;
		return true;
	}

	void TriggerSequenceSystem::Clear()
	{
		_sequences.clear();
	}

	TriggerSequenceSystem::Sequence* TriggerSequenceSystem::FindSequence(const std::wstring& sequenceId)
	{
		const auto iter = _sequences.find(sequenceId);
		if (iter == _sequences.end() || iter->second.actions.empty())
			return nullptr;

		return &iter->second;
	}

	const TriggerSequenceSystem::Sequence* TriggerSequenceSystem::FindSequence(const std::wstring& sequenceId) const
	{
		const auto iter = _sequences.find(sequenceId);
		if (iter == _sequences.end() || iter->second.actions.empty())
			return nullptr;

		return &iter->second;
	}

	void TriggerSequenceSystem::ActivateAll()
	{
		for (const auto& sequencePair : _sequences)
			Activate(sequencePair.first);
	}

	void TriggerSequenceSystem::ResetAll()
	{
		for (const auto& sequencePair : _sequences)
			Reset(sequencePair.first);
	}
}
