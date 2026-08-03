#pragma once

namespace gm
{
	class ChiMoveComponent;
	class ChiStateMachineComponent;
	class Rigidbody3DComponent;
	class SkeletalAnimatorComponent;

	struct ChiStateContext
	{
		ChiStateMachineComponent*	stateMachine = nullptr;
		ChiMoveComponent*			moveComponent = nullptr;
		Rigidbody3DComponent*		rigidbodyComponent = nullptr;
		SkeletalAnimatorComponent*	animatorComponent = nullptr;
	};
}
