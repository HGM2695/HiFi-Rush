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

	class ChiDashState : public ChiState
	{
	public:
		ChiDashState(ChiStateId stateId, ChiAnimationClipId animationClipId, ChiDashDirection direction, bool rotateToDashDirection, float runTransitionStartBeat, ChiStateId nextDashState = ChiStateId::None, float cameraDistanceOffset = 0.f, float landingEffectBeat = -1.f, float landingEffectForwardOffset = 0.f);

		virtual void Enter(ChiStateContext& context) override;
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
		virtual void Exit(ChiStateContext& context) override;

	protected:
		bool			TryChangeDashAttack(ChiStateContext& context);
		Vector3			GetDashDirection(ChiStateContext& context) const;

	private:
		ChiDashDirection					_direction = ChiDashDirection::Front;
		ChiStateId							_nextDashState = ChiStateId::None;
		Vector3								_cachedDirection{};
		bool								_rotateToDashDirection = false;
		float								_runTransitionStartBeat = 0.f;
		float								_cameraDistanceOffset = 0.f;
		float								_landingEffectBeat = -1.f;
		float								_landingEffectForwardOffset = 0.f;
		bool								_hasSpawnedLandingEffect = false;
		std::optional<RhythmJudgeResult>	_bufferedAttackInput;
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
		void Enter(ChiStateContext& context) override;
		void Tick(ChiStateContext& context, float deltaTime) override;

	private:
		uint32 _nextCrescentIndex = 0;
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
	class ChiDashSkyFallState final : public ChiState
	{
	public:
		ChiDashSkyFallState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};
}
