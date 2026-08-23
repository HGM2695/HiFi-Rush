#pragma once

#include "ChiAnimationTypes.h"
#include "Component.h"
#include "EffectSpawner.h"
#include "WeakGameObjectPtr.h"

#include <array>
#include <optional>
#include <vector>

namespace gm
{
	class CameraFollowComponent;
	class ChiAfterImageObject;
	class EffectPresets;
	class Resources;
	class SkeletalMeshComponent;

	class ChiEffectComponent final : public Component
	{
	public:
		ChiEffectComponent(Resources& resources, const EffectPresets& effectPresets);
		TickGroup GetTickGroup() const override { return TickGroup::Attachment; }

		void SpawnJumpEffect();
		void RequestFingerSnapEffect();
		void SpawnLandingEffect();
		void SpawnDoubleJumpEffect();
		void SpawnDashEffect(const Vector3& dashDirection, float cameraDistanceOffset);
		void SpawnDoubleDashCrescent(uint32 crescentIndex);
		void SpawnDashLandingEffect(float forwardOffset);
		void EndDashEffect();
		void SetAfterImageEmissiveIntensity(float intensity);
		float GetAfterImageEmissiveIntensity() const { return _afterImageEmissiveIntensity; }
		void SetAfterImageColor(uint32 index, const Color& color);
		Color GetAfterImageColor(uint32 index) const;
		void SetAfterImageOpacity(float opacity);
		float GetAfterImageOpacity() const { return _afterImageOpacity; }
		void SetAfterImageColorBlendRatio(float ratio);
		float GetAfterImageColorBlendRatio() const { return _afterImageColorBlendRatio; }
		void SpawnAttackEffect(ChiAnimationClipId animationClipId);
		void SpawnBeatHitEffect(ChiAnimationClipId animationClipId, const Matrix& world);
		void SpawnStumpEffect();
		void SpawnHibikiCloud();
		void SpawnHibikiExitClouds();

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		void StartAfterImageTrail();
		void SpawnPendingFingerSnapEffect();
		void UpdateAfterImages();
		void StopAfterImageTrail();
		ChiAfterImageObject* GetAfterImage(uint32 index) const;
		void StartDashCamera(float distanceOffset);
		CameraFollowComponent* FindCameraFollowComponent() const;

		struct AfterImageSample
		{
			float elapsed = 0.f;
			Matrix world = Matrix::Identity;
		};

		EffectSpawner			_effectSpawner;
		SkeletalMeshComponent* _skeletalMeshComponent = nullptr;
		std::array<WeakGameObjectPtr, 3> _afterImages{};
		std::array<Color, 3> _afterImageColors = { Color{ 0.f, 1.f, 0.f, 0.2f }, Color{ 1.f, 0.f, 0.f, 0.2f }, Color{ 0.f, 0.f, 1.f, 0.2f } };
		float						_afterImageOpacity = 0.2f;
		float						_afterImageColorBlendRatio = 0.8f;
		std::vector<AfterImageSample> _afterImageHistory{};
		float					_afterImageElapsed = 0.f;
		float					_afterImageEmissiveIntensity = 0.2f;
		CameraFollowComponent* _cameraFollowComponent = nullptr;
		std::optional<float>	_dashCameraBaseDistance{};
		bool					_isAfterImageTrailActive = false;
		bool					_isFingerSnapEffectPending = false;
	};
}
