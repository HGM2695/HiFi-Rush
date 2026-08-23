#include "GunnerEffectComponent.h"

#include "EffectPresets.h"
#include "GameObject.h"
#include "MathUtil.h"
#include "Resources.h"
#include "Scene.h"
#include "SocketComponent.h"
#include "TransformComponent.h"

#include <algorithm>
#include <cmath>

namespace gm
{
	namespace
	{
		constexpr wchar_t GroundReadyLaserGuideEffectId[] = L"Gunner.LaserGuide.Ground.Ready";
		constexpr wchar_t GroundAttackLaserGuideEffectId[] = L"Gunner.LaserGuide.Ground.Attack";
		constexpr wchar_t SkyReadyLaserGuideEffectId[] = L"Gunner.LaserGuide.Sky.Ready";
		constexpr wchar_t SkyAttackLaserGuideEffectId[] = L"Gunner.LaserGuide.Sky.Attack";
		constexpr wchar_t LaserEffectId[] = L"Gunner.Laser";
		constexpr wchar_t LaserSocketName[] = L"Gunner.Laser";
		constexpr float GroundGuideMinimumLength = 14.f;
		constexpr float GuideExtraLength = 3.f;
		constexpr float GroundGuideHeightOffset = 0.02f;
		constexpr float SkyGuideWidth = 0.02f;

		bool CreateGroundGuideWorld(const Vector3& start, const Vector3& target, float halfWidth, _Out_ Matrix& outWorld)
		{
			Vector3 offset = target - start;
			const float targetDistance = offset.Length();
			if (targetDistance <= 0.000001f || halfWidth <= 0.f)
				return false;

			const Vector3 direction = offset / targetDistance;
			Vector3 right = Vector3::Up.Cross(direction);
			if (right.LengthSquared() <= 0.000001f)
				right = Vector3::Right;
			right.Normalize();
			Vector3 normal = direction.Cross(right);
			normal.Normalize();
			const float length = std::max(targetDistance + GuideExtraLength, GroundGuideMinimumLength);
			const Vector3 center = start + direction * (length * 0.5f) + normal * GroundGuideHeightOffset;
			outWorld = Matrix{ right.x * halfWidth * 2.f, right.y * halfWidth * 2.f, right.z * halfWidth * 2.f, 0.f, direction.x * length, direction.y * length, direction.z * length, 0.f, normal.x, normal.y, normal.z, 0.f, center.x, center.y, center.z, 1.f };
			return true;
		}

		bool CreateSkyGuideWorld(const Vector3& start, const Vector3& target, _Out_ Matrix& outWorld)
		{
			Vector3 offset = target - start;
			const float targetDistance = offset.Length();
			if (targetDistance <= 0.000001f)
				return false;

			const Vector3 direction = offset / targetDistance;
			Vector3 right = Vector3::Up.Cross(direction);
			if (right.LengthSquared() <= 0.000001f)
				right = Vector3::Right;
			right.Normalize();
			Vector3 look = right.Cross(direction);
			look.Normalize();
			const float length = targetDistance + GuideExtraLength;
			const Vector3 center = start + direction * (length * 0.5f);
			outWorld = Matrix{ right.x * SkyGuideWidth, right.y * SkyGuideWidth, right.z * SkyGuideWidth, 0.f, direction.x * length, direction.y * length, direction.z * length, 0.f, look.x, look.y, look.z, 0.f, center.x, center.y, center.z, 1.f };
			return true;
		}
	}

	GunnerEffectComponent::GunnerEffectComponent(Resources& resources, const EffectPresets& effectPresets)
		: _effectSpawner(resources, effectPresets)
	{
	}

	void GunnerEffectComponent::StartGroundReadyLaserGuide()
	{
		StartLaserGuide(GroundReadyLaserGuideEffectId);
	}

	void GunnerEffectComponent::StartGroundAttackLaserGuide()
	{
		StartLaserGuide(GroundAttackLaserGuideEffectId);
	}

	void GunnerEffectComponent::StartSkyReadyLaserGuide()
	{
		StartLaserGuide(SkyReadyLaserGuideEffectId);
	}

	void GunnerEffectComponent::StartSkyAttackLaserGuide()
	{
		StartLaserGuide(SkyAttackLaserGuideEffectId);
	}

	bool GunnerEffectComponent::UpdateGroundLaserGuide(const Vector3& start, const Vector3& target, float halfWidth)
	{
		Matrix world{};
		return CreateGroundGuideWorld(start, target, halfWidth, world) && _laserGuide.SetWorldTransform(world);
	}

	bool GunnerEffectComponent::UpdateSkyLaserGuide(const Vector3& start, const Vector3& target)
	{
		Matrix world{};
		return CreateSkyGuideWorld(start, target, world) && _laserGuide.SetWorldTransform(world);
	}

	void GunnerEffectComponent::StopLaserGuide()
	{
		_laserGuide.Stop();
	}

	bool GunnerEffectComponent::SpawnLaser(const Vector3& start, const Vector3& direction, float length) const
	{
		Scene* scene = GetOwner().GetScene();
		GM_ASSERT_RETURN_VAL(scene, false, "Gunner Laser Effect를 생성할 Scene이 없습니다.");
		GM_ASSERT_RETURN_VAL(direction.LengthSquared() > 0.000001f && length > 0.f, false, "Gunner Laser Effect 방향 또는 길이가 유효하지 않습니다.");
		const Vector3 up = std::abs(direction.Dot(Vector3::Up)) > 0.99f ? Vector3::Right : Vector3::Up;
		const Matrix orientation = Math::CreateLookAtLH(start, start + direction, up).Invert();
		return _effectSpawner.SpawnAtWorld(*scene, LaserEffectId, Matrix::CreateScale(1.f, 1.f, length) * orientation);
	}

	Vector3 GunnerEffectComponent::GetLaserSocketPosition() const
	{
		GM_ASSERT_RETURN_VAL(_socketComponent && _socketComponent->HasSocket(LaserSocketName), GetOwner().GetTransform()->GetPosition(), "Gunner Laser Socket이 없습니다.");
		return Vector3::Transform(Vector3::Zero, _socketComponent->GetSocketAnchorWorldMatrix(LaserSocketName));
	}

	void GunnerEffectComponent::OnInitialize()
	{
		_socketComponent = GetOwner().GetComponent<SocketComponent>();
		GM_ASSERT_RETURN(_socketComponent && _socketComponent->HasSocket(LaserSocketName), "GunnerEffectComponent에는 Gunner Laser Socket이 필요합니다.");
	}

	bool GunnerEffectComponent::StartLaserGuide(const std::wstring& effectId)
	{
		Scene* scene = GetOwner().GetScene();
		GM_ASSERT_RETURN_VAL(scene, false, "Gunner Laser Guide를 생성할 Scene이 없습니다.");
		StopLaserGuide();
		return _effectSpawner.SpawnAtWorld(*scene, effectId, Matrix::Identity, _laserGuide);
	}
}
