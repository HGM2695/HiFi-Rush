#pragma once

#include "WeakGameObjectPtr.h"
#if GM_ENABLE_DEBUG_TOOLS
#include "Event.h"
#endif

#include <string>
#include <unordered_map>
#include <vector>

namespace gm
{
	class BeatSystem;
	class IBeatTriggerAction;

	class TriggerSequenceSystem
	{
	public:
		explicit TriggerSequenceSystem(const BeatSystem& beatSystem);

		bool RegisterAction(const std::wstring& sequenceId, float beatOffset, const WeakGameObjectPtr& actionOwner, IBeatTriggerAction& actionComponent);
		bool HasSequence(const std::wstring& sequenceId) const;
		bool Activate(const std::wstring& sequenceId);
		bool Reset(const std::wstring& sequenceId);
		void Clear();

	private:
		struct ActionEntry
		{
			WeakGameObjectPtr	actionOwner{};
			IBeatTriggerAction*	actionComponent = nullptr;
			float				beatOffset = 0.f;
		};

		struct Sequence
		{
			std::vector<ActionEntry>	actions{};
			bool						isActivated = false;
		};

		Sequence* FindSequence(const std::wstring& sequenceId);
		const Sequence* FindSequence(const std::wstring& sequenceId) const;
		void ActivateAll();
		void ResetAll();

	private:
		const BeatSystem&							_beatSystem;
		std::unordered_map<std::wstring, Sequence>	_sequences{};

#if GM_ENABLE_DEBUG_TOOLS
		EventConnection _debugEventConnection{};
#endif
	};
}
