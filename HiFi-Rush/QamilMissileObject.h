#pragma once

#include "GameObject.h"
#include "WeakGameObjectPtr.h"

#include <memory>

namespace gm
{
	class EffectSpawner;
	class SkeletalAnimationClip;
	class SkeletalAnimatorComponent;
	class SkeletalMesh;
	class Texture;

	struct QamilMissileDesc
	{
		Vector3 startPosition{};
		Vector3 initialDirection{ 0.f, 0.f, 1.f };
		Vector3 targetDirection{ 0.f, -1.f, 0.f };
		Vector3 warningPosition{};
		Vector3 arenaCenter{};
		WeakGameObjectPtr target{};
		std::shared_ptr<SkeletalMesh> skeletalMesh{};
		std::shared_ptr<SkeletalAnimationClip> animation{};
		std::shared_ptr<Texture> warningTexture{};
	};

	class QamilMissileObject final : public GameObject
	{
	public:
		explicit QamilMissileObject(const QamilMissileDesc& desc);
		~QamilMissileObject() override;

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		void UpdatePreparation(float elapsedBeat);
		void UpdateWarningPosition(float deltaTime);
		void BeginLaunch();
		void UpdateLaunch(float currentBeat);
		bool SpawnCreationEffect() const;
		bool SpawnAttachedSmoke();
		bool SpawnTrailEffect(const Vector3& position) const;
		bool SpawnTrailEffects(const Vector3& startPosition, const Vector3& endPosition);
		bool SpawnExplosionEffect() const;
		void Explode();
		void DestroyWarning();

		WeakGameObjectPtr _target{};
		WeakGameObjectPtr _warning{};
		std::shared_ptr<Texture> _warningTexture{};
		std::unique_ptr<EffectSpawner> _effectSpawner{};
		SkeletalAnimatorComponent* _animatorComponent = nullptr;
		Vector3 _initialPosition{};
		Vector3 _preparationEndPosition{};
		Vector3 _warningPosition{};
		Vector3 _arenaCenter{};
		Vector3 _launchStartPosition{};
		Vector3 _launchEndPosition{};
		Quaternion _initialRotation{};
		Quaternion _targetRotation{};
		float _spawnBeat = 0.f;
		float _launchStartBeat = 0.f;
		float _trailDistanceSinceLastSpawn = 0.f;
		bool _isLaunchStarted = false;
	};
}
