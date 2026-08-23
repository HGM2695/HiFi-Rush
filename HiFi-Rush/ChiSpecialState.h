#pragma once

#include "ChiAttackState.h"

#include <optional>

namespace gm
{
	class CameraFollowComponent;

	/// Hibiki //////////////////////////////////////////////////////////////////////////////
	class ChiHibikiReadyState final : public ChiState
	{
	public:
		ChiHibikiReadyState();
		virtual void Enter(ChiStateContext& context) override;
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
		virtual void Exit(ChiStateContext& context) override;
	};

	class ChiHibikiAttackState final : public ChiAttackState
	{
	public:
		ChiHibikiAttackState();
		virtual void Enter(ChiStateContext& context) override;
		virtual void Tick(ChiStateContext& context, float deltaTime) override;
		virtual void Exit(ChiStateContext& context) override;

	private:
		virtual bool UsesAutoTargeting() const override { return false; }
		CameraFollowComponent* FindCameraFollowComponent(ChiStateContext& context) const;
		void RestoreCamera();
		bool SpawnAreaHitBox(ChiStateContext& context) const;

		CameraFollowComponent*	_cameraFollowComponent = nullptr;
		std::optional<float>	_previousCameraTargetDistance{};
		bool					_hasStartedCameraMove = false;
		bool					_hasSpawnedAreaHitBox = false;
		bool					_hasSpawnedExitClouds = false;
	};
}
