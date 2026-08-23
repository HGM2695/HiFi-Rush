#include "QamilEffectComponent.h"

#include "CameraManager.h"
#include "CombatTypes.h"
#include "EffectInstance.h"
#include "EffectPresets.h"
#include "GameObject.h"
#include "HealthComponent.h"
#include "HiFiRushAudio.h"
#include "MathUtil.h"
#include "Scene.h"
#include "TransformComponent.h"

#include <cmath>

namespace gm
{
	namespace
	{
		constexpr wchar_t QamilHitEffectId[] = L"Combat.Hit";
		constexpr wchar_t QamilPunchImpactCircleEffectId[] = L"Qamil.PunchImpact.Circle";
		constexpr wchar_t QamilPunchImpactSmokeEffectId[] = L"Qamil.PunchImpact.Smoke";
		constexpr wchar_t QamilPunchImpactBronzeStarEffectId[] = L"Qamil.PunchImpact.BronzeStar";
		constexpr wchar_t QamilStumpEdgeImpactEffectId[] = L"Qamil.Stump.EdgeImpact";
		constexpr wchar_t QamilStumpFloorEffectId[] = L"Qamil.Stump.Floor";
		constexpr wchar_t QamilSweepWarningEffectId[] = L"Qamil.Sweep.Warning";
		constexpr wchar_t QamilSweepClockwiseEffectId[] = L"Qamil.Sweep.Clockwise";
		constexpr wchar_t QamilSweepCounterClockwiseEffectId[] = L"Qamil.Sweep.CounterClockwise";
		constexpr wchar_t QamilLaserStrikeEffectId[] = L"Qamil.Laser.Strike";
		constexpr wchar_t QamilLaserRestoreSmokeEffectId[] = L"Qamil.Laser.RestoreSmoke";
		constexpr float QamilPunchImpactHeightOffset = -0.5f;
		constexpr float QamilPunchImpactSmokeCameraOffset = 0.05f;
		constexpr float QamilPunchImpactBronzeStarCameraOffset = -0.1f;
		constexpr float QamilStumpEdgeDistance = 12.5f;
		constexpr float QamilStumpEdgeHeight = 1.f;
		constexpr float QamilStumpFloorHeight = 0.2f;
		constexpr float QamilSweepWarningHeight = 0.05f;
		constexpr float QamilSweepBaseDirectionAngle = 2.35619449f;

		bool CreateSweepWorld(const TransformComponent& transform, const Vector3& attackCenter, float heightOffset, Matrix& outWorld)
		{
			const Vector3 arenaCenter = transform.GetPosition();
			Vector3 attackDirection = attackCenter - arenaCenter;
			attackDirection.y = 0.f;
			if (attackDirection.LengthSquared() <= 0.f)
				return false;

			attackDirection.Normalize();
			const float attackDirectionAngle = std::atan2(attackDirection.x, attackDirection.z);
			Vector3 effectPosition = arenaCenter;
			effectPosition.y += heightOffset;
			outWorld = Matrix::CreateRotationY(attackDirectionAngle - QamilSweepBaseDirectionAngle) * Matrix::CreateTranslation(effectPosition);
			return true;
		}
	}

	QamilEffectComponent::QamilEffectComponent(Resources& resources, const EffectPresets& effectPresets)
		: _effectSpawner(resources, effectPresets)
	{
	}

	bool QamilEffectComponent::SpawnPunchImpact(const Vector3& handCenter, float handRadius) const
	{
		Scene* scene = GetOwner().GetScene();
		CameraManager* cameraManager = scene ? scene->GetCameraManager() : nullptr;
		GM_ASSERT_RETURN_VAL(cameraManager && cameraManager->GetActiveCamera(), false, "Qamil Punch Impact를 생성할 Scene 또는 Camera가 없습니다.");

		Vector3 cameraDirection = cameraManager->GetViewInfo().position - handCenter;
		cameraDirection.y = 0.f;
		if (cameraDirection.LengthSquared() > 0.f)
			cameraDirection.Normalize();

		Vector3 effectPosition = handCenter + cameraDirection * handRadius;
		effectPosition.y += QamilPunchImpactHeightOffset;
		const bool hasSpawnedCircle = _effectSpawner.SpawnAtWorld(*scene, QamilPunchImpactCircleEffectId, Matrix::CreateTranslation(effectPosition));
		const bool hasSpawnedSmoke = _effectSpawner.SpawnAtWorld(*scene, QamilPunchImpactSmokeEffectId, Matrix::CreateTranslation(effectPosition + cameraDirection * QamilPunchImpactSmokeCameraOffset));
		const bool hasSpawnedBronzeStar = _effectSpawner.SpawnAtWorld(*scene, QamilPunchImpactBronzeStarEffectId, Matrix::CreateTranslation(effectPosition + cameraDirection * QamilPunchImpactBronzeStarCameraOffset));
		return hasSpawnedCircle && hasSpawnedSmoke && hasSpawnedBronzeStar;
	}

	bool QamilEffectComponent::SpawnStump(const Vector3& platformPosition) const
	{
		Scene* scene = GetOwner().GetScene();
		const TransformComponent* transform = GetOwner().GetTransform();
		GM_ASSERT_RETURN_VAL(scene && transform, false, "Qamil Stump Effect를 생성할 Scene 또는 Transform이 없습니다.");

		const Vector3 arenaCenter = transform->GetPosition();
		Vector3 platformDirection = platformPosition - arenaCenter;
		platformDirection.y = 0.f;
		GM_ASSERT_RETURN_VAL(platformDirection.LengthSquared() > 0.f, false, "Qamil Stump Effect의 Platform 방향이 유효하지 않습니다.");
		platformDirection.Normalize();

		Vector3 edgePosition = arenaCenter + platformDirection * QamilStumpEdgeDistance;
		edgePosition.y += QamilStumpEdgeHeight;
		Vector3 floorPosition = arenaCenter;
		floorPosition.y += QamilStumpFloorHeight;
		const bool hasSpawnedEdgeImpact = _effectSpawner.SpawnAtWorld(*scene, QamilStumpEdgeImpactEffectId, Matrix::CreateTranslation(edgePosition));
		const bool hasSpawnedFloor = _effectSpawner.SpawnAtWorld(*scene, QamilStumpFloorEffectId, Matrix::CreateTranslation(floorPosition));
		return hasSpawnedEdgeImpact && hasSpawnedFloor;
	}

	bool QamilEffectComponent::SpawnSweepWarning(const Vector3& attackCenter, EffectInstance& outInstance) const
	{
		Scene* scene = GetOwner().GetScene();
		const TransformComponent* transform = GetOwner().GetTransform();
		GM_ASSERT_RETURN_VAL(scene && transform, false, "Qamil Sweep Warning을 생성할 Scene 또는 Transform이 없습니다.");
		Matrix world;
		GM_ASSERT_RETURN_VAL(CreateSweepWorld(*transform, attackCenter, QamilSweepWarningHeight, world), false, "Qamil Sweep Warning의 공격 방향이 유효하지 않습니다.");
		return _effectSpawner.SpawnAtWorld(*scene, QamilSweepWarningEffectId, world, outInstance);
	}

	bool QamilEffectComponent::SpawnSweep(const Vector3& attackCenter, bool isClockwise) const
	{
		Scene* scene = GetOwner().GetScene();
		const TransformComponent* transform = GetOwner().GetTransform();
		GM_ASSERT_RETURN_VAL(scene && transform, false, "Qamil Sweep Effect를 생성할 Scene 또는 Transform이 없습니다.");

		Matrix world;
		GM_ASSERT_RETURN_VAL(CreateSweepWorld(*transform, attackCenter, 0.f, world), false, "Qamil Sweep Effect의 공격 방향이 유효하지 않습니다.");
		return _effectSpawner.SpawnAtWorld(*scene, isClockwise ? QamilSweepClockwiseEffectId : QamilSweepCounterClockwiseEffectId, world);
	}

	bool QamilEffectComponent::SpawnLaserStrike(const Vector3& handCenter, const Vector3& direction) const
	{
		Scene* scene = GetOwner().GetScene();
		GM_ASSERT_RETURN_VAL(scene, false, "Qamil Laser Effect를 생성할 Scene이 없습니다.");
		GM_ASSERT_RETURN_VAL(direction.LengthSquared() > 0.000001f, false, "Qamil Laser Effect 방향이 유효하지 않습니다.");
		Vector3 normalizedDirection = direction;
		normalizedDirection.Normalize();
		const Vector3 up = std::abs(normalizedDirection.Dot(Vector3::Up)) > 0.99f ? Vector3::Right : Vector3::Up;
		const Matrix world = Math::CreateLookAtLH(handCenter, handCenter + normalizedDirection, up).Invert();
		return _effectSpawner.SpawnAtWorld(*scene, QamilLaserStrikeEffectId, world);
	}

	bool QamilEffectComponent::SpawnLaserRestoreSmoke(const Vector3& leftHandCenter, const Vector3& rightHandCenter) const
	{
		Scene* scene = GetOwner().GetScene();
		GM_ASSERT_RETURN_VAL(scene, false, "Qamil Laser Restore Smoke를 생성할 Scene이 없습니다.");
		const bool hasSpawnedLeftSmoke = _effectSpawner.SpawnAtWorld(*scene, QamilLaserRestoreSmokeEffectId, Matrix::CreateTranslation(leftHandCenter));
		const bool hasSpawnedRightSmoke = _effectSpawner.SpawnAtWorld(*scene, QamilLaserRestoreSmokeEffectId, Matrix::CreateTranslation(rightHandCenter));
		return hasSpawnedLeftSmoke && hasSpawnedRightSmoke;
	}

	void QamilEffectComponent::OnInitialize()
	{
		HealthComponent* healthComponent = GetOwner().GetComponent<HealthComponent>();
		GM_ASSERT_RETURN(healthComponent, "QamilEffectComponent는 HealthComponent가 필요합니다.");
		healthComponent->OnDamaged.Subscribe(_damagedConnection, [this](const HitEvent& event) { HandleDamaged(event); });
	}

	void QamilEffectComponent::HandleDamaged(const HitEvent& event)
	{
		if (event.damageResult.state == DamageState::Ignored || event.hurtBox == nullptr)
			return;

		if (event.damageResult.state == DamageState::Applied)
			PlayRandomSound2D(HiFiRushSound::MonsterHitImpacts);

		Scene* scene = GetOwner().GetScene();
		GM_ASSERT_RETURN(scene, "Qamil Hit Effect를 생성할 Scene이 없습니다.");
		GM_ASSERT(_effectSpawner.SpawnAtWorld(*scene, QamilHitEffectId, Matrix::CreateTranslation(event.contact.selfPoint)), "Qamil Hit Effect 생성에 실패했습니다.");
	}
}
