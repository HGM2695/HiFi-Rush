#pragma once

#include "ChiState.h"

namespace gm
{
	/// Jump //////////////////////////////////////////////////////////////////////////////
	class ChiJumpUpState final : public ChiClipState
	{
	public:
		ChiJumpUpState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	class ChiJumpDownState final : public ChiClipState
	{
	public:
		ChiJumpDownState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	class ChiJumpLandingState final : public ChiClipState
	{
	public:
		ChiJumpLandingState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	/// DoubleJump //////////////////////////////////////////////////////////////////////////////
	class ChiJumpDoubleUpState final : public ChiClipState
	{
	public:
		ChiJumpDoubleUpState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};

	class ChiJumpDoubleDownState final : public ChiClipState
	{
	public:
		ChiJumpDoubleDownState();
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
	};
}
