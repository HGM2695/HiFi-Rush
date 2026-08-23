#include "QamilMissileObject.h"

#include "Application.h"
#include "AudioStatics.h"
#include "BeatMath.h"
#include "BeatSystem.h"
#include "EffectPresets.h"
#include "EffectSpawner.h"
#include "HiFiRushCollisionLayers.h"
#include "HiFiRushAudio.h"
#include "HiFiRushStatics.h"
#include "MathUtil.h"
#include "Scene.h"
#include "SkeletalAnimationClip.h"
#include "SkeletalAnimatorComponent.h"
#include "SkeletalMesh.h"
#include "SkeletalMeshComponent.h"
#include "SpriteComponent.h"
#include "TemporaryHitBoxObject.h"
#include "Texture.h"
#include "TransformComponent.h"

#include <algorithm>
#include <cmath>

namespace gm
{
	namespace
	{
		constexpr wchar_t QamilMissileAnimationName[] = L"Qamil.Missile.Idle";
		constexpr float QamilMissilePreparationBeats = 2.f;
		constexpr float QamilMissileLaunchDelayBeats = 8.f;
		constexpr float QamilMissileLaunchDurationBeats = 1.f;
		constexpr float QamilMissilePreparationDistance = 5.f;
		constexpr float QamilMissileWarningMoveSpeed = 3.f;
		constexpr float QamilMissileWarningRadius = 3.f;
		constexpr float QamilArenaRadius = 14.8f;
		constexpr float QamilMissilePulseScale = 0.5f;
		constexpr float QamilMissileAnimationTicksPerBeat = 15.f;
		constexpr int32 QamilMissileDamage = 30;
		constexpr float QamilMissileHitBoxLifetime = 0.05f;
		constexpr float QamilMissileTrailSpacing = 0.5f;
		constexpr float QamilMissileCreationEffectHeight = 1.f;
		constexpr float QamilMissileExplosionEffectHeight = 1.5f;
		constexpr wchar_t QamilMissileCreationEffectId[] = L"Qamil.Missile.Creation";
		constexpr wchar_t QamilMissileAttachedSmokeEffectId[] = L"Qamil.Missile.AttachedSmoke";
		constexpr wchar_t QamilMissileTrailEffectId[] = L"Qamil.Missile.Trail";
		constexpr wchar_t QamilMissileExplosionEffectId[] = L"Qamil.Missile.Explosion";

		Quaternion CreateLookRotation(const Vector3& direction)
		{
			Vector3 normalizedDirection = direction;
			if (normalizedDirection.LengthSquared() <= 0.000001f)
				return Quaternion{ 0.f, 0.f, 0.f, 1.f };
			normalizedDirection.Normalize();
			const Vector3 up = std::abs(normalizedDirection.Dot(Vector3::Up)) > 0.99f ? Vector3::Right : Vector3::Up;
			Matrix world = Math::CreateLookAtLH(Vector3{}, normalizedDirection, up).Invert();
			Vector3 scale{};
			Quaternion rotation{};
			Vector3 position{};
			world.Decompose(scale, rotation, position);
			return rotation;
		}
	}

	QamilMissileObject::QamilMissileObject(const QamilMissileDesc& desc)
		: _target(desc.target)
		, _warningTexture(desc.warningTexture)
		, _initialPosition(desc.startPosition)
		, _warningPosition(desc.warningPosition)
		, _arenaCenter(desc.arenaCenter)
	{
		GM_ASSERT_RETURN(desc.skeletalMesh && desc.animation && desc.warningTexture, "Qamil Missile Resource가 유효하지 않습니다.");
		Vector3 initialDirection = desc.initialDirection;
		GM_ASSERT_RETURN(initialDirection.LengthSquared() > 0.000001f, "Qamil Missile 초기 방향이 유효하지 않습니다.");
		initialDirection.Normalize();
		_preparationEndPosition = _initialPosition + initialDirection * QamilMissilePreparationDistance;
		_initialRotation = CreateLookRotation(initialDirection);
		_targetRotation = CreateLookRotation(desc.targetDirection);
		GetTransform()->SetPosition(_initialPosition);
		GetTransform()->SetRotation(_initialRotation);

		SkeletalMeshComponent* meshComponent = AddComponent<SkeletalMeshComponent>();
		GM_ASSERT_RETURN(meshComponent, "Qamil Missile SkeletalMeshComponent 생성에 실패했습니다.");
		meshComponent->SetSkeletalMesh(desc.skeletalMesh);
		_animatorComponent = AddComponent<SkeletalAnimatorComponent>();
		GM_ASSERT_RETURN(_animatorComponent, "Qamil Missile SkeletalAnimatorComponent 생성에 실패했습니다.");
		GM_ASSERT_RETURN(_animatorComponent->AddClip(QamilMissileAnimationName, desc.animation), "Qamil Missile Animation 등록에 실패했습니다.");
	}

	QamilMissileObject::~QamilMissileObject()
	{
		DestroyWarning();
	}

	void QamilMissileObject::OnInitialize()
	{
		_effectSpawner = std::make_unique<EffectSpawner>(APPLICATION.GetResources(), HiFiRushStatics::GetEffectPresets());
		const BeatSystem& beatSystem = HiFiRushStatics::GetBeatSystem();
		GM_ASSERT_RETURN(beatSystem.HasPlaybackTime(), "Qamil Missile은 재생 중인 BeatSystem이 필요합니다.");
		GM_ASSERT_RETURN(_animatorComponent && _animatorComponent->Play(QamilMissileAnimationName, AnimationPlayOption{ .loopOverride = true }), "Qamil Missile Animation 재생에 실패했습니다.");
		const std::shared_ptr<SkeletalAnimationClip> clip = _animatorComponent->GetCurrentClip();
		if (clip && clip->GetTicksPerSecond() > 0.f && beatSystem.GetSecondsPerBeat() > 0.f)
			_animatorComponent->SetPlayRate((QamilMissileAnimationTicksPerBeat / clip->GetTicksPerSecond()) / beatSystem.GetSecondsPerBeat());
		_spawnBeat = static_cast<float>(beatSystem.GetCurrentBeatIndex());

		Scene* scene = GetScene();
		GM_ASSERT_RETURN(scene, "Qamil Missile Warning을 생성할 Scene이 없습니다.");
		GameObject* warning = scene->SpawnGameObject<GameObject>();
		GM_ASSERT_RETURN(warning, "Qamil Missile Warning GameObject 생성에 실패했습니다.");
		warning->GetTransform()->SetPosition(_warningPosition);
		warning->GetTransform()->SetRotationEuler(Vector3{ Math::GM_PI * 0.5f, 0.f, 0.f });
		warning->GetTransform()->SetScale(Vector3{ QamilMissileWarningRadius * 2.f, QamilMissileWarningRadius * 2.f, 1.f });
		SpriteComponent* spriteComponent = warning->AddComponent<SpriteComponent>();
		GM_ASSERT_RETURN(spriteComponent, "Qamil Missile Warning SpriteComponent 생성에 실패했습니다.");
		spriteComponent->SetTexture(_warningTexture);
		_warning = warning->GetWeakPtr();
		GM_ASSERT(SpawnCreationEffect(), "Qamil Missile 생성 Effect 생성에 실패했습니다.");
	}

	void QamilMissileObject::OnTick(float deltaTime)
	{
		const BeatSystem& beatSystem = HiFiRushStatics::GetBeatSystem();
		if (beatSystem.HasPlaybackTime() == false)
			return;

		const float currentBeat = beatSystem.GetCurrentBeat();
		const float elapsedBeat = currentBeat - _spawnBeat;
		if (_isLaunchStarted == false)
		{
			if (elapsedBeat < QamilMissilePreparationBeats)
				UpdatePreparation(elapsedBeat);
			if (elapsedBeat < QamilMissileLaunchDelayBeats)
			{
				UpdateWarningPosition(deltaTime);
				const float pulse = BeatMath::EvaluateBeatIntervalPulse(currentBeat, 1.f);
				GetTransform()->SetScale(Vector3{ 1.f, 1.f, 1.f } * (1.f + pulse * QamilMissilePulseScale));
				return;
			}
			BeginLaunch();
		}

		UpdateLaunch(currentBeat);
	}

	void QamilMissileObject::UpdatePreparation(float elapsedBeat)
	{
		const float ratio = std::clamp(elapsedBeat / QamilMissilePreparationBeats, 0.f, 1.f);
		const float smoothRatio = ratio * ratio * (3.f - 2.f * ratio);
		GetTransform()->SetPosition(Vector3::Lerp(_initialPosition, _preparationEndPosition, smoothRatio));
		GetTransform()->SetRotation(Quaternion::Slerp(_initialRotation, _targetRotation, smoothRatio));
	}

	void QamilMissileObject::UpdateWarningPosition(float deltaTime)
	{
		GameObject* target = _target.Get();
		GameObject* warning = _warning.Get();
		if (target == nullptr || target->GetTransform() == nullptr || warning == nullptr || warning->GetTransform() == nullptr)
			return;

		Vector3 direction = target->GetTransform()->GetPosition() - _warningPosition;
		direction.y = 0.f;
		const float distance = direction.Length();
		if (distance <= 0.000001f)
			return;
		direction /= distance;
		const float movementDistance = (std::min)(distance, QamilMissileWarningMoveSpeed * (std::max)(deltaTime, 0.f));
		Vector3 nextPosition = _warningPosition + direction * movementDistance;
		Vector3 centerOffset = Math::ProjectOnXZPlane(nextPosition - _arenaCenter);
		const float maximumRadius = QamilArenaRadius - QamilMissileWarningRadius;
		if (centerOffset.LengthSquared() > maximumRadius * maximumRadius)
		{
			centerOffset.Normalize();
			nextPosition.x = _arenaCenter.x + centerOffset.x * maximumRadius;
			nextPosition.z = _arenaCenter.z + centerOffset.z * maximumRadius;
		}
		_warningPosition = nextPosition;
		warning->GetTransform()->SetPosition(_warningPosition);
	}

	void QamilMissileObject::BeginLaunch()
	{
		_isLaunchStarted = true;
		_launchStartBeat = _spawnBeat + QamilMissileLaunchDelayBeats;
		_launchStartPosition = GetTransform()->GetPosition();
		_launchEndPosition = Vector3{ _warningPosition.x, _arenaCenter.y, _warningPosition.z };
		GetTransform()->SetScale(Vector3{ 1.f, 1.f, 1.f });
		GetTransform()->SetRotation(CreateLookRotation(_launchEndPosition - _launchStartPosition));
		_trailDistanceSinceLastSpawn = 0.f;
		GM_ASSERT(SpawnTrailEffect(_launchStartPosition), "Qamil Missile 첫 번째 추적 Effect 생성에 실패했습니다.");
		GM_ASSERT(SpawnAttachedSmoke(), "Qamil Missile 부착 Smoke 생성에 실패했습니다.");
	}

	void QamilMissileObject::UpdateLaunch(float currentBeat)
	{
		const float ratio = std::clamp((currentBeat - _launchStartBeat) / QamilMissileLaunchDurationBeats, 0.f, 1.f);
		const Vector3 currentPosition = GetTransform()->GetPosition();
		const Vector3 nextPosition = Vector3::Lerp(_launchStartPosition, _launchEndPosition, ratio);
		GM_ASSERT(SpawnTrailEffects(currentPosition, nextPosition), "Qamil Missile 추적 Effect 생성에 실패했습니다.");
		GetTransform()->SetPosition(nextPosition);
		if (ratio >= 1.f)
			Explode();
	}

	bool QamilMissileObject::SpawnCreationEffect() const
	{
		Scene* scene = GetScene();
		if (scene == nullptr || _effectSpawner == nullptr)
			return false;
		Vector3 position = _initialPosition;
		position.y += QamilMissileCreationEffectHeight;
		return _effectSpawner->SpawnAtWorld(*scene, QamilMissileCreationEffectId, Matrix::CreateTranslation(position));
	}

	bool QamilMissileObject::SpawnAttachedSmoke()
	{
		Scene* scene = GetScene();
		return scene && _effectSpawner && _effectSpawner->SpawnAttachedToOwner(*scene, QamilMissileAttachedSmokeEffectId, *this);
	}

	bool QamilMissileObject::SpawnTrailEffect(const Vector3& position) const
	{
		Scene* scene = GetScene();
		return scene && _effectSpawner && _effectSpawner->SpawnAtWorld(*scene, QamilMissileTrailEffectId, Matrix::CreateTranslation(position));
	}

	bool QamilMissileObject::SpawnTrailEffects(const Vector3& startPosition, const Vector3& endPosition)
	{
		Vector3 direction = endPosition - startPosition;
		float remainingDistance = direction.Length();
		if (remainingDistance <= 0.f)
			return true;

		direction /= remainingDistance;
		Vector3 cursor = startPosition;
		float distanceUntilSpawn = QamilMissileTrailSpacing - _trailDistanceSinceLastSpawn;
		while (remainingDistance >= distanceUntilSpawn)
		{
			cursor += direction * distanceUntilSpawn;
			if (SpawnTrailEffect(cursor) == false)
				return false;
			remainingDistance -= distanceUntilSpawn;
			distanceUntilSpawn = QamilMissileTrailSpacing;
			_trailDistanceSinceLastSpawn = 0.f;
		}
		_trailDistanceSinceLastSpawn += remainingDistance;
		return true;
	}

	bool QamilMissileObject::SpawnExplosionEffect() const
	{
		Scene* scene = GetScene();
		if (scene == nullptr || _effectSpawner == nullptr)
			return false;
		Vector3 position = _launchEndPosition;
		position.y += QamilMissileExplosionEffectHeight;
		return _effectSpawner->SpawnAtWorld(*scene, QamilMissileExplosionEffectId, Matrix::CreateTranslation(position));
	}

	void QamilMissileObject::Explode()
	{
		GM_ASSERT(SpawnExplosionEffect(), "Qamil Missile 폭발 Effect 생성에 실패했습니다.");
		Scene* scene = GetScene();
		if (scene)
		{
			TemporarySphereHitBoxDesc desc{};
			desc.world = Matrix::CreateTranslation(_launchEndPosition);
			desc.colliderId = L"Qamil.Missile";
			desc.radius = QamilMissileWarningRadius;
			desc.collisionLayer = HiFiRushCollisionLayer::MonsterAttack;
			desc.collisionMask = HiFiRushCollisionLayer::Player;
			desc.damageInfo.amount = QamilMissileDamage;
			desc.damageInfo.hitReactionType = HitReactionType::StrongKnockback;
			desc.lifetime = QamilMissileHitBoxLifetime;
			GM_ASSERT(scene->SpawnGameObject<TemporaryHitBoxObject>(desc), "Qamil Missile 폭발 HitBox 생성에 실패했습니다.");
		}
		PlaySound2D(HiFiRushSound::QamilMissileExplosion);
		DestroyWarning();
		Destroy();
	}

	void QamilMissileObject::DestroyWarning()
	{
		GameObject* warning = _warning.Get();
		if (warning)
			warning->Destroy();
		_warning.Reset();
	}
}
