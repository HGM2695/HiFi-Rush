#pragma once

#include "ChiAttackState.h"

namespace gm
{
	/// Hibiki //////////////////////////////////////////////////////////////////////////////
	class ChiHibikiReadyState final : public ChiState
	{
	public:
		ChiHibikiReadyState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	class ChiHibikiAttackState final : public ChiAttackState
	{
	public:
		ChiHibikiAttackState();
		virtual void Enter(ChiStateContext& context) override;
		virtual void Tick(ChiStateContext& context, float deltaTime) override;

	private:
		virtual bool UsesAutoTargeting() const override { return false; }
		bool SpawnAreaHitBox(ChiStateContext& context) const;

		bool _hasSpawnedAreaHitBox = false;
	};
}
