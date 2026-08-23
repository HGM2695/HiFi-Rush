#pragma once

#include "ChiState.h"
#include "Event.h"

namespace gm
{
	struct AnimationNotifyEvent;

	/// Idle //////////////////////////////////////////////////////////////////////////////
	class ChiIdleState final : public ChiState
	{
	public:
		ChiIdleState();
		virtual void		Enter(ChiStateContext& context) override;
		virtual void		Tick(ChiStateContext& context, float deltaTime) override;
		virtual void		Exit(ChiStateContext& context) override;

	private:
		void HandleAnimationNotify(ChiStateContext& context, const AnimationNotifyEvent& event);

		EventConnection _fingerSnapNotifyConnection{};
	};

	/// Run //////////////////////////////////////////////////////////////////////////////
	class ChiRunState final : public ChiState
	{
	public:
		ChiRunState();
		virtual void		Enter(ChiStateContext& context) override;
		virtual void		Tick(ChiStateContext& context, float deltaTime) override;

	private:
		int64	_lastFootstepBeatIndex = -1;
		size_t	_footstepIndex = 0;
	};
}
