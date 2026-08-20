#include "PlayerTargetingComponent.h"

#include "CameraComponent.h"
#include "Collider3DComponent.h"
#include "CollisionTypes.h"
#include "CombatTargetComponent.h"
#include "GameObject.h"
#include "HiFiRushCollisionLayers.h"
#include "MathUtil.h"
#include "PhysicsSystem3D.h"
#include "Scene.h"
#include "TransformComponent.h"

#include <cmath>
#include <limits>
#include <unordered_set>
#include <utility>

namespace gm
{
	PlayerTargetingComponent::PlayerTargetingComponent(const PhysicsSystem3D& physicsSystem, const CameraComponent& referenceCamera, PlayerTargetingDesc desc)
		: _physicsSystem(physicsSystem), _referenceCamera(&referenceCamera), _referenceCameraOwner(referenceCamera.GetOwner().GetWeakPtr()), _desc(std::move(desc))
	{
		GM_ASSERT(_desc.findRadius > 0.f, "자동 타겟 획득 반경은 0보다 커야 합니다.");
		GM_ASSERT(_desc.releaseRadius >= _desc.findRadius, "자동 타겟 해제 반경은 획득 반경 이상이어야 합니다.");
		GM_ASSERT(_desc.closeTargetRadius >= 0.f && _desc.closeTargetRadius <= _desc.findRadius, "근접 타겟 반경은 0 이상이고 획득 반경 이하여야 합니다.");
		GM_ASSERT(_desc.viewAngle > 0.f && _desc.viewAngle <= 180.f, "자동 타겟 허용 각도는 0도 초과 180도 이하여야 합니다.");
		GM_ASSERT(_desc.directionWeight >= 0.f, "자동 타겟 방향 가중치는 0 이상이어야 합니다.");
		GM_ASSERT(_desc.currentTargetBonus >= 0.f, "현재 타겟 유지 보너스는 0 이상이어야 합니다.");
	}

	GameObject* PlayerTargetingComponent::AcquireTarget(const Vector3& direction)
	{
		Scene* scene = GetOwner().GetScene();
		if (scene == nullptr || _ownerTransform == nullptr)
		{
			ClearTarget();
			return nullptr;
		}

		const Vector3 findDirection = GetFindDirection(direction);
		CombatTargetComponent* bestTarget = nullptr;
		uint32 bestTargetPointIndex = 0;
		float bestTargetCost = (std::numeric_limits<float>::max)();

		if (_target.IsValid() && _targetComponent && _targetPointIndex)
		{
			if (CanTargetPoint(*_targetComponent, _targetPointIndex.value(), findDirection, _desc.releaseRadius, false))
			{
				bestTarget = _targetComponent;
				bestTargetPointIndex = _targetPointIndex.value();
				bestTargetCost = EvaluateTargetPointCost(*_targetComponent, bestTargetPointIndex, findDirection) - _desc.currentTargetBonus;
			}
			else
			{
				ClearTarget();
			}
		}
		else
		{
			ClearTarget();
		}

		CollisionQueryFilter filter{};
		filter.mask = HiFiRushCollisionLayer::Monster;
		filter.includeTriggers = true;
		const std::vector<Collider3DComponent*> overlaps = _physicsSystem.OverlapSphere(*scene, _ownerTransform->GetPosition(), _desc.findRadius, filter);

		std::unordered_set<CombatTargetComponent*> evaluatedTargets;
		for (Collider3DComponent* collider : overlaps)
		{
			if (collider == nullptr)
				continue;

			CombatTargetComponent* candidate = collider->GetOwner().GetComponent<CombatTargetComponent>();
			if (candidate == nullptr || evaluatedTargets.insert(candidate).second == false)
				continue;

			for (uint32 targetPointIndex = 0; targetPointIndex < candidate->GetTargetPointCount(); ++targetPointIndex)
			{
				if (candidate == _targetComponent && _targetPointIndex && targetPointIndex == _targetPointIndex.value())
					continue;
				if (CanTargetPoint(*candidate, targetPointIndex, findDirection, _desc.findRadius, true) == false)
					continue;

				const float targetCost = EvaluateTargetPointCost(*candidate, targetPointIndex, findDirection);
				if (targetCost >= bestTargetCost)
					continue;

				bestTarget = candidate;
				bestTargetPointIndex = targetPointIndex;
				bestTargetCost = targetCost;
			}
		}

		SetTarget(bestTarget, bestTargetPointIndex);
		return GetTarget();
	}

	GameObject* PlayerTargetingComponent::GetTarget() const
	{
		if (_target.IsValid() == false || _targetComponent == nullptr || _targetPointIndex.has_value() == false || _targetComponent->IsTargetPointTargetable(_targetPointIndex.value()) == false)
			return nullptr;

		return _target.GetUnsafe();
	}

	Vector3 PlayerTargetingComponent::GetTargetPosition() const
	{
		if (GetTarget() == nullptr)
			return {};

		return _targetComponent->GetTargetPosition(_targetPointIndex.value());
	}

	Vector3 PlayerTargetingComponent::GetTargetDirection() const
	{
		if (GetTarget() == nullptr || _ownerTransform == nullptr)
			return {};

		return Math::GetNormalizedXZDirection(GetTargetPosition() - _ownerTransform->GetPosition());
	}

	void PlayerTargetingComponent::ClearTarget()
	{
		_target.Reset();
		_targetComponent = nullptr;
		_targetPointIndex.reset();
	}

	void PlayerTargetingComponent::OnInitialize()
	{
		_ownerTransform = GetOwner().GetTransform();
		GM_ASSERT_RETURN(_ownerTransform, "PlayerTargetingComponent는 TransformComponent가 필요합니다.");
	}

	Vector3 PlayerTargetingComponent::GetFindDirection(const Vector3& preferredDirection) const
	{
		Vector3 findDirection = Math::GetNormalizedXZDirection(preferredDirection);
		if (findDirection.LengthSquared() > 0.000001f)
			return findDirection;

		if (_referenceCameraOwner.IsValid() && _referenceCamera)
		{
			findDirection = Math::GetNormalizedXZDirection(_referenceCamera->GetForwardDirection());
			if (findDirection.LengthSquared() > 0.000001f)
				return findDirection;
		}

		if (_ownerTransform)
			return Math::GetNormalizedXZDirection(Math::GetLookVector(_ownerTransform->GetRotation()));

		return Vector3{ 0.f, 0.f, 1.f };
	}

	bool PlayerTargetingComponent::CanTargetPoint(const CombatTargetComponent& target, uint32 targetPointIndex, const Vector3& findDirection, float maxDistance, bool checkTargetAngle) const
	{
		if (target.IsTargetPointTargetable(targetPointIndex) == false || _ownerTransform == nullptr)
			return false;

		const Vector3 targetOffset = target.GetTargetPosition(targetPointIndex) - _ownerTransform->GetPosition();
		const float distance = targetOffset.Length();
		if (distance > maxDistance)
			return false;

		const Vector3 targetDirection = Math::GetNormalizedXZDirection(targetOffset);
		const float directionDot = findDirection.Dot(targetDirection);
		const float minimumDot = std::cos(Math::DegreesToRadians(_desc.viewAngle));
		if (checkTargetAngle && directionDot < minimumDot && distance > _desc.closeTargetRadius)
			return false;

		return true;
	}

	float PlayerTargetingComponent::EvaluateTargetPointCost(const CombatTargetComponent& target, uint32 targetPointIndex, const Vector3& findDirection) const
	{
		const Vector3 targetOffset = target.GetTargetPosition(targetPointIndex) - _ownerTransform->GetPosition();
		const float distance = targetOffset.Length();
		const Vector3 targetDirection = Math::GetNormalizedXZDirection(targetOffset);
		const float directionDot = findDirection.Dot(targetDirection);
		return distance + (1.f - directionDot) * _desc.directionWeight;
	}

	void PlayerTargetingComponent::SetTarget(CombatTargetComponent* target, uint32 targetPointIndex)
	{
		if (target == nullptr)
		{
			ClearTarget();
			return;
		}

		_target = target->GetOwner().GetWeakPtr();
		_targetComponent = target;
		_targetPointIndex = targetPointIndex;
	}
}
