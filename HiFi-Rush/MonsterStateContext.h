#pragma once

#include "CombatTypes.h"

namespace gm
{
	class BeatSystem;
	class HealthComponent;
	class MonsterCombatComponent;
	class CharacterMovementComponent;
	class MonsterStateMachineComponent;
	class Rigidbody3DComponent;
	class SkeletalAnimatorComponent;

	// 몬스터 상태 클래스가 사용할 컴포넌트 묶음
	struct MonsterStateContext
	{
		const BeatSystem*				beatSystem = nullptr;
		MonsterStateMachineComponent*	stateMachine = nullptr;
		MonsterCombatComponent*			combatComponent = nullptr;
		CharacterMovementComponent*		moveComponent = nullptr;
		Rigidbody3DComponent*			rigidbodyComponent = nullptr;
		HealthComponent*				healthComponent = nullptr;
		SkeletalAnimatorComponent*		animatorComponent = nullptr;
		HitReactionType					lastHitReactionType = HitReactionType::None;
	};
}
