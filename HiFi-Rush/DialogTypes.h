#pragma once

#include "Event.h"
#include "PlayerControlTypes.h"

#include <string>
#include <vector>

namespace gm
{
	enum class DialogPortrait
	{
		Chi,
		Announcer,
		Saver,

		Count
	};

	enum class DialogPlaybackState
	{
		Idle,
		PlayingVoice,
		WaitingForLineEnd,
		WaitingForBranch,
	};

	enum class DialogControlReleasePoint
	{
		SequenceFinished,
		BranchRequested,
	};

	struct DialogBranchData
	{
		std::wstring	key{};
		std::wstring	nextLineId{};
	};

	struct DialogLineData
	{
		std::wstring					id{};
		std::wstring					speakerName{};
		std::wstring					text{};
		DialogPortrait					portrait = DialogPortrait::Announcer;

		std::wstring					voiceResourceKey{};
		float							voiceVolume = 1.f;

		std::wstring					soundEffectResourceKey{};
		float							soundEffectVolume = 1.f;

		std::wstring					nextLineId{};
		std::vector<DialogBranchData>	branches{};
	};

	struct DialogSequenceData
	{
		std::wstring					id{};
		std::wstring					entryLineId{};
		std::vector<DialogLineData>		lines{};
		PlayerControlMask				blockedPlayerControls = PlayerControl::None;
		PlayerControlMask				blockedPlayerControlsAfterRelease = PlayerControl::None;
		DialogControlReleasePoint		controlReleasePoint = DialogControlReleasePoint::SequenceFinished;
	};

	struct DialogLineChangedEvent final : EventType
	{
		const DialogLineData* line = nullptr;
	};

	struct DialogBranchRequestedEvent final : EventType
	{
		const DialogSequenceData*	sequence = nullptr;
		const DialogLineData*		line = nullptr;
	};

	struct DialogFinishedEvent final : EventType
	{
		std::wstring	sequenceId{};
		bool			completed = false;
	};
}
