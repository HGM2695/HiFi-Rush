#pragma once

#include "RhythmInputJudge.h"

#include <optional>

namespace gm
{
	class BeatSystem;
	class ChiMoveComponent;
	class ChiAnimationSettings;
	class ChiStateMachineComponent;
	class RhythmJudge;
	class Rigidbody3DComponent;
	class SkeletalAnimatorComponent;

	struct ChiStateContext
	{
		ChiStateMachineComponent*			stateMachine = nullptr;
		BeatSystem*							beatSystem = nullptr;
		const ChiAnimationSettings*			animationSettings = nullptr;
		ChiMoveComponent*					moveComponent = nullptr;
		Rigidbody3DComponent*				rigidbodyComponent = nullptr;
		SkeletalAnimatorComponent*			animatorComponent = nullptr;
		std::optional<float>				blendDuration;

		const RhythmJudge*					rhythmJudge = nullptr;
		std::optional<RhythmJudgeResult>	weakAttackInput;
		std::optional<RhythmJudgeResult>	strongAttackInput;
		std::optional<RhythmJudgeResult>	jumpInput;
		std::optional<RhythmJudgeResult>	dashInput;
		std::optional<RhythmJudgeResult>	transitionRhythmInput;
	};
}
