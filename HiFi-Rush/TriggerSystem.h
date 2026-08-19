#pragma once

#include "Event.h"

#include <string>
#include <unordered_set>

namespace gm
{
	class BeatSystem;
	enum class TriggerType
	{
		Activate,
		Reset,
	};

	struct TriggerEvent : EventType
	{
		TriggerType		type = TriggerType::Activate;
		std::wstring	triggerId{};
		float			startBeat = 0.f;
		bool			targetsAll = false;
	};

	class TriggerSystem
	{
	public:
		explicit TriggerSystem(const BeatSystem& beatSystem);

		bool Activate(const std::wstring& triggerId);
		bool Reset(const std::wstring& triggerId);
		void Clear();

		EventPublisher<TriggerSystem, TriggerEvent> OnTrigger;

	private:
		void ActivateAll();
		void ResetAll();
		void PublishTrigger(TriggerType type, const std::wstring& triggerId, float startBeat, bool targetsAll);

	private:
		const BeatSystem&					_beatSystem;
		std::unordered_set<std::wstring>	_activatedTriggerIds{};

#if GM_ENABLE_DEBUG_TOOLS
		EventConnection _debugEventConnection{};
#endif
	};
}
