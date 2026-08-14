#pragma once

#include "MonsterState.h"

namespace gm
{
	class SjangoState : public MonsterState
	{
	public:
		void OnDamaged(MonsterStateContext& context, const HitEvent& event) override;
	};

	// Idle /////////////////////////////////////////////////////////////////////////
	class SjangoIdleState final : public SjangoState
	{
	public:
		MonsterStateId	GetStateId() const override { return MonsterStateId::Idle; }
		void			Enter(MonsterStateContext& context) override;
	};

	// Damage /////////////////////////////////////////////////////////////////////////
	class SjangoDamageState final : public SjangoState
	{
	public:
		MonsterStateId	GetStateId() const override { return MonsterStateId::Damage; }
		void			Enter(MonsterStateContext& context) override;
		void			Tick(MonsterStateContext& context, float deltaTime) override;
	};
}
