#pragma once

#include "MathTypes.h"
#include "RhythmInputJudge.h"

#include <optional>

namespace gm
{
	class BeatSystem;
	class ChiMoveComponent;
	class ChiAnimationSettings;
	class ChiStateMachineComponent;
	class PlayerTargetingComponent;
	class ReverbComponent;
	class RhythmJudge;
	class Rigidbody3DComponent;
	class SkeletalAnimatorComponent;

	struct ChiStateContext
	{
		ChiStateMachineComponent*			stateMachine = nullptr;
		BeatSystem*							beatSystem = nullptr;
		const ChiAnimationSettings*			animationSettings = nullptr;
		ChiMoveComponent*					moveComponent = nullptr;
		PlayerTargetingComponent*			targetingComponent = nullptr;
		ReverbComponent*					reverbComponent = nullptr;
		Rigidbody3DComponent*				rigidbodyComponent = nullptr;
		SkeletalAnimatorComponent*			animatorComponent = nullptr;
		std::optional<float>				blendDuration;
		std::optional<Vector3>				airborneImpulse;

		const RhythmJudge*					rhythmJudge = nullptr;
		std::optional<RhythmJudgeResult>	weakAttackInput;
		std::optional<RhythmJudgeResult>	strongAttackInput;
		std::optional<RhythmJudgeResult>	jumpInput;
		std::optional<RhythmJudgeResult>	dashInput;
		std::optional<RhythmJudgeResult>	beatHitInput;
		std::optional<RhythmJudgeResult>	transitionRhythmInput;
	};
}
