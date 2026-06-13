#pragma once

#include "ChiState.h"

namespace gm
{
	/// Damage //////////////////////////////////////////////////////////////////////////////
	class ChiDamageState : public ChiClipState
	{
	public:
		ChiDamageState(ChiStateId stateId, ChiAnimationId animationId, bool returnToBaseMotion);

		virtual void Tick(ChiStateContext& context, float deltaTime) override;

	private:
		bool _returnToBaseMotion = true;
	};

	class ChiWeakKnockbackDamageState final : public ChiDamageState
	{
	public:
		ChiWeakKnockbackDamageState();
	};

	class ChiStrongKnockbackDamageState final : public ChiDamageState
	{
	public:
		ChiStrongKnockbackDamageState();
	};

	class ChiDeadDamageState final : public ChiDamageState
	{
	public:
		ChiDeadDamageState();
	};
}
