#pragma once

#include "ChiState.h"

namespace gm
{
	/// Idle //////////////////////////////////////////////////////////////////////////////
	class ChiIdleState final : public ChiState
	{
	public:
		virtual ChiStateId GetStateId() const override { return ChiStateId::Idle; }
		virtual void Enter(ChiStateContext& context) override;
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	/// Run //////////////////////////////////////////////////////////////////////////////
	class ChiRunState final : public ChiState
	{
	public:
		virtual ChiStateId GetStateId() const override { return ChiStateId::Run; }
		virtual void Enter(ChiStateContext& context) override;
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};
}
