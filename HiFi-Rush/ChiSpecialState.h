#pragma once

#include "ChiState.h"

namespace gm
{
	/// Hibiki //////////////////////////////////////////////////////////////////////////////
	class ChiHibikiReadyState final : public ChiClipState
	{
	public:
		ChiHibikiReadyState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	class ChiHibikiAttackState final : public ChiClipState
	{
	public:
		ChiHibikiAttackState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};
}
