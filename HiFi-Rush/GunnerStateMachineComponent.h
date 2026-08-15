#pragma once

#include "MonsterStateMachineComponent.h"

namespace gm
{
	class GunnerStateMachineComponent final : public MonsterStateMachineComponent
	{
	public:
		GunnerStateMachineComponent(float attackRangeMin, float attackRangeMax, int32 attackDamage);

	protected:
		void OnInitialize() override;

	private:
		bool RegisterAnimationClips();
		bool RegisterStates();

		float _attackRangeMin = 0.f;
		float _attackRangeMax = 0.f;
		int32 _attackDamage = 0;
	};
}
