#pragma once

#include "ChiState.h"

namespace gm
{
	/// Idle //////////////////////////////////////////////////////////////////////////////
	class ChiIdleState final : public ChiState
	{
	public:
		ChiIdleState();
		virtual void		Tick(ChiStateContext& context, float deltaTime) override;
	};

	/// Run //////////////////////////////////////////////////////////////////////////////
	class ChiRunState final : public ChiState
	{
	public:
		ChiRunState();
		virtual void		Tick(ChiStateContext& context, float deltaTime) override;
	};
}
