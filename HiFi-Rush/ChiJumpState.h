#pragma once

#include "ChiState.h"

namespace gm
{
	/// Jump //////////////////////////////////////////////////////////////////////////////
	class ChiJumpUpState final : public ChiState
	{
	public:
		ChiJumpUpState();
		virtual void Enter(ChiStateContext& context) override;
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
		virtual void Exit(ChiStateContext& context) override;
	};

	class ChiJumpDownState final : public ChiState
	{
	public:
		ChiJumpDownState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	class ChiJumpLandingState final : public ChiState
	{
	public:
		ChiJumpLandingState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	/// DoubleJump //////////////////////////////////////////////////////////////////////////////
	class ChiJumpDoubleUpState final : public ChiState
	{
	public:
		ChiJumpDoubleUpState();
		virtual void Enter(ChiStateContext& context) override;
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
		virtual void Exit(ChiStateContext& context) override;
	};

	class ChiJumpDoubleDownState final : public ChiState
	{
	public:
		ChiJumpDoubleDownState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};
}
