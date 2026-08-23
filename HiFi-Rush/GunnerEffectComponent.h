#pragma once

#include "Component.h"
#include "EffectInstance.h"
#include "EffectSpawner.h"

namespace gm
{
	class EffectPresets;
	class Resources;
	class SocketComponent;

	class GunnerEffectComponent final : public Component
	{
	public:
		GunnerEffectComponent(Resources& resources, const EffectPresets& effectPresets);

		void StartGroundReadyLaserGuide();
		void StartGroundAttackLaserGuide();
		void StartSkyReadyLaserGuide();
		void StartSkyAttackLaserGuide();
		bool UpdateGroundLaserGuide(const Vector3& start, const Vector3& target, float halfWidth);
		bool UpdateSkyLaserGuide(const Vector3& start, const Vector3& target);
		void StopLaserGuide();
		bool SpawnLaser(const Vector3& start, const Vector3& direction, float length) const;
		Vector3 GetLaserSocketPosition() const;

	protected:
		void OnInitialize() override;

	private:
		bool StartLaserGuide(const std::wstring& effectId);

		EffectSpawner		_effectSpawner;
		EffectInstance		_laserGuide{};
		SocketComponent*	_socketComponent = nullptr;
	};
}
