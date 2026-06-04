#pragma once

namespace gm
{
	class ChiMoveComponent;
	class ChiStateMachineComponent;
	class SkeletalAnimatorComponent;

	struct ChiStateContext
	{
		ChiStateMachineComponent*	stateMachine = nullptr;
		ChiMoveComponent*			moveComponent = nullptr;
		SkeletalAnimatorComponent*	animatorComponent = nullptr;
	};
}
