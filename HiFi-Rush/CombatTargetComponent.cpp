#include "CombatTargetComponent.h"

#include "GameObject.h"
#include "HealthComponent.h"
#include "TransformComponent.h"

namespace gm
{
	CombatTargetComponent::CombatTargetComponent(const Vector3& localTargetOffset)
		: _localTargetOffset(localTargetOffset)
	{
	}

	void CombatTargetComponent::OnInitialize()
	{
		_ownerTransform = GetOwner().GetTransform();
		GM_ASSERT_RETURN(_ownerTransform, "CombatTargetComponent는 TransformComponent가 필요합니다.");

		_healthComponent = GetOwner().GetComponent<HealthComponent>();
		GM_ASSERT_RETURN(_healthComponent, "CombatTargetComponent는 HealthComponent가 필요합니다.");
	}

	bool CombatTargetComponent::IsTargetable() const
	{
		return IsEnabled() && _healthComponent && _healthComponent->IsEnabled() && _healthComponent->IsDead() == false && GetOwner().IsPendingDestroy() == false;
	}

	Vector3 CombatTargetComponent::GetTargetPosition() const
	{
		if (_ownerTransform == nullptr)
			return {};

		return Vector3::Transform(_localTargetOffset, _ownerTransform->GetWorldMatrix());
	}
}
