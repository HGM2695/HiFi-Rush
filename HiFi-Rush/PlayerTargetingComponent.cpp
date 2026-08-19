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
		float bestTargetCost = (std::numeric_limits<float>::max)();

		if (_target.IsValid() && _targetComponent)
		{
			if (canTargeting(*_targetComponent, findDirection, _desc.releaseRadius, false))
			{
				bestTarget = _targetComponent;
				bestTargetCost = EvaluateTargetCost(*_targetComponent, findDirection) - _desc.currentTargetBonus;
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
		const std::vector<Collider3DComponent*> overlaps = _physicsSystem.OverlapSphere(*scene, _ownerTransform->GetPosition(), _desc.findRadius, filter);

		std::unordered_set<CombatTargetComponent*> evaluatedTargets;
		for (Collider3DComponent* collider : overlaps)
		{
			if (collider == nullptr)
				continue;

			CombatTargetComponent* candidate = collider->GetOwner().GetComponent<CombatTargetComponent>();
			if (candidate == nullptr || evaluatedTargets.insert(candidate).second == false || candidate == _targetComponent)
				continue;

			if (canTargeting(*candidate, findDirection, _desc.findRadius, true) == false)
				continue;

			const float targetCost = EvaluateTargetCost(*candidate, findDirection);
			if (targetCost >= bestTargetCost)
				continue;

			bestTarget = candidate;
			bestTargetCost = targetCost;
		}

		SetTarget(bestTarget);
		return GetTarget();
	}

	GameObject* PlayerTargetingComponent::GetTarget() const
	{
		if (_target.IsValid() == false || _targetComponent == nullptr || _targetComponent->IsTargetable() == false)
			return nullptr;

		return _target.GetUnsafe();
	}

	Vector3 PlayerTargetingComponent::GetTargetPosition() const
	{
		if (GetTarget() == nullptr)
			return {};

		return _targetComponent->GetTargetPosition();
	}

	Vector3 PlayerTargetingComponent::GetTargetDirection() const
	{
		if (GetTarget() == nullptr || _ownerTransform == nullptr)
			return {};

		return Math::GetNormalizedXZDirection(_targetComponent->GetTargetPosition() - _ownerTransform->GetPosition());
	}

	void PlayerTargetingComponent::ClearTarget()
	{
		_target.Reset();
		_targetComponent = nullptr;
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

	bool PlayerTargetingComponent::canTargeting(const CombatTargetComponent& target, const Vector3& findDirection, float maxDistance, bool isCheckTargetAngle) const
	{
		if (target.IsTargetable() == false || _ownerTransform == nullptr)
			return false;

		const Vector3 targetOffset = target.GetTargetPosition() - _ownerTransform->GetPosition();
		const float distance = targetOffset.Length();
		if (distance > maxDistance)
			return false;

		const Vector3 targetDirection = Math::GetNormalizedXZDirection(targetOffset);
		const float directionDot = findDirection.Dot(targetDirection);
		const float minimumDot = std::cos(Math::DegreesToRadians(_desc.viewAngle));
		if (isCheckTargetAngle && directionDot < minimumDot && distance > _desc.closeTargetRadius)
			return false;

		return true;
	}

	float PlayerTargetingComponent::EvaluateTargetCost(const CombatTargetComponent& target, const Vector3& findDirection) const
	{
		const Vector3 targetOffset = target.GetTargetPosition() - _ownerTransform->GetPosition();
		const float distance = targetOffset.Length();
		const Vector3 targetDirection = Math::GetNormalizedXZDirection(targetOffset);
		const float directionDot = findDirection.Dot(targetDirection);
		return distance + (1.f - directionDot) * _desc.directionWeight;
	}

	void PlayerTargetingComponent::SetTarget(CombatTargetComponent* target)
	{
		if (target == nullptr)
		{
			ClearTarget();
			return;
		}

		_target = target->GetOwner().GetWeakPtr();
		_targetComponent = target;
	}
}
