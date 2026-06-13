#pragma once

#include "ChiState.h"

namespace gm
{
	/// Dash //////////////////////////////////////////////////////////////////////////////
	enum class ChiDashDirection
	{
		Front,
		Back,
		Left,
		Right,
		InputOrFront,
	};

	class ChiDashState : public ChiClipState
	{
	public:
		ChiDashState(ChiStateId stateId, ChiAnimationId animationId, ChiDashDirection direction, float dashSpeed, bool rotateToDashDirection, ChiStateId nextDashState = ChiStateId::None);

		virtual void Enter(ChiStateContext& context) override;
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
		virtual void Exit(ChiStateContext& context) override;

	protected:
		bool TryChangeDashAttack(ChiStateContext& context) const;
		Vector3 GetDashDirection(ChiStateContext& context) const;
		const Vector3& GetCachedDirection() const { return _cachedDirection; }
		float GetDashSpeed() const { return _dashSpeed; }

	private:
		ChiDashDirection	_direction = ChiDashDirection::Front;
		ChiStateId			_nextDashState = ChiStateId::None;
		Vector3				_cachedDirection{};
		float				_dashSpeed = 0.f;
		bool				_rotateToDashDirection = false;
		bool				_prevMoveEnabled = true;
	};

	class ChiDashFrontState final : public ChiDashState
	{
	public:
		ChiDashFrontState();
	};

	class ChiDashBackState final : public ChiDashState
	{
	public:
		ChiDashBackState();
	};

	class ChiDashLeftState final : public ChiDashState
	{
	public:
		ChiDashLeftState();
	};

	class ChiDashRightState final : public ChiDashState
	{
	public:
		ChiDashRightState();
	};

	class ChiDashDoubleState final : public ChiDashState
	{
	public:
		ChiDashDoubleState();
	};

	class ChiDashTripleState final : public ChiDashState
	{
	public:
		ChiDashTripleState();
	};

	/// DashSky //////////////////////////////////////////////////////////////////////////////
	class ChiDashSkyState final : public ChiDashState
	{
	public:
		ChiDashSkyState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	/// DashSkyFall //////////////////////////////////////////////////////////////////////////////
	class ChiDashSkyFallState final : public ChiClipState
	{
	public:
		ChiDashSkyFallState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};
}
