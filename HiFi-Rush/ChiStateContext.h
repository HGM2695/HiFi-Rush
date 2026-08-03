#pragma once

namespace gm
{
	class ChiMoveComponent;
	class ChiStateMachineComponent;
	class NavMeshControllerComponent;
	class Rigidbody3DComponent;
	class SkeletalAnimatorComponent;

	struct ChiStateContext
	{
		ChiStateMachineComponent*	stateMachine = nullptr;
		ChiMoveComponent*			moveComponent = nullptr;
		NavMeshControllerComponent*	navMeshControllerComponent = nullptr;
		Rigidbody3DComponent*		rigidbodyComponent = nullptr;
		SkeletalAnimatorComponent*	animatorComponent = nullptr;
	};
}
