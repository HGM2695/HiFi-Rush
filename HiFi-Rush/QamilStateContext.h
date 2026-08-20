#pragma once

#include "WeakGameObjectPtr.h"

namespace gm
{
	class BeatSystem;
	class HealthComponent;
	class QamilStateMachineComponent;
	class SkeletalAnimatorComponent;
	class SkeletalMeshComponent;
	class SocketComponent;
	class TransformComponent;
	class TriggerSystem;

	struct QamilStateContext
	{
		const BeatSystem*				beatSystem = nullptr;
		QamilStateMachineComponent*		stateMachine = nullptr;
		HealthComponent*				healthComponent = nullptr;
		SkeletalAnimatorComponent*		animatorComponent = nullptr;
		SkeletalMeshComponent*			skeletalMeshComponent = nullptr;
		SocketComponent*				socketComponent = nullptr;
		TransformComponent*			transformComponent = nullptr;
		TriggerSystem*				triggerSystem = nullptr;
		WeakGameObjectPtr				target{};
	};
}
