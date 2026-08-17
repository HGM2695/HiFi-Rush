#pragma once

#include "ChiState.h"

namespace gm
{
	/// Damage //////////////////////////////////////////////////////////////////////////////
	class ChiDamageState : public ChiState
	{
	public:
		ChiDamageState(ChiStateId stateId, ChiAnimationClipId animationClipId, bool returnToBaseMotion);

		virtual void Tick(ChiStateContext& context, float deltaTime) override;
		virtual void OnGroundContact(ChiStateContext& context, const NavigationGroundContactEvent& event) override;
		virtual void OnGroundLost(ChiStateContext& context, const NavigationGroundLostEvent& event) override;

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
