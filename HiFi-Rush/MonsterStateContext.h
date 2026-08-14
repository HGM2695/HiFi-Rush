#pragma once

namespace gm
{
	class HealthComponent;
	class MonsterCombatComponent;
	class CharacterMovementComponent;
	class MonsterStateMachineComponent;
	class SkeletalAnimatorComponent;

	// 몬스터 상태 클래스가 사용할 컴포넌트 묶음
	struct MonsterStateContext
	{
		MonsterStateMachineComponent*	stateMachine = nullptr;
		MonsterCombatComponent*			combatComponent = nullptr;
		CharacterMovementComponent*	moveComponent = nullptr;
		HealthComponent*				healthComponent = nullptr;
		SkeletalAnimatorComponent*		animatorComponent = nullptr;
	};
}
