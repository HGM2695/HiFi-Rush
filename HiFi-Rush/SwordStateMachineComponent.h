#pragma once

#include "MonsterStateMachineComponent.h"

namespace gm
{
	class HitBoxComponent;

	class SwordStateMachineComponent final : public MonsterStateMachineComponent
	{
	public:
		SwordStateMachineComponent(float attackRangeMin, float attackRangeMax);

	protected:
		void OnInitialize() override;

	private:
		bool RegisterAnimationClips();
		bool RegisterStates(HitBoxComponent& hitBox);

	private:
		float _attackRangeMin = 0.f;
		float _attackRangeMax = 0.f;
	};
}
