#pragma once

#include "ChiState.h"

namespace gm
{
	/// Hibiki //////////////////////////////////////////////////////////////////////////////
	class ChiHibikiReadyState final : public ChiState
	{
	public:
		ChiHibikiReadyState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	class ChiHibikiAttackState final : public ChiState
	{
	public:
		ChiHibikiAttackState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};
}
