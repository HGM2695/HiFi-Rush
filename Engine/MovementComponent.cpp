#include "MovementComponent.h"

#include "GameObject.h"
#include "NavMeshControllerComponent.h"
#include "TransformComponent.h"

namespace gm
{
	MovementComponent::MovementComponent() = default;
	MovementComponent::~MovementComponent() = default;

	void MovementComponent::EnableNavigationMovement(bool enabled)
	{
		GM_ASSERT_RETURN(_navMeshController || enabled == false, "Navigation 이동을 활성화하려면 NavMeshControllerComponent가 필요합니다.");
		if (_navMeshController)
			_navMeshController->SetMovementEnabled(enabled);
	}

	bool MovementComponent::IsNavigationMovementEnabled() const
	{
		return _navMeshController && _navMeshController->IsMovementEnabled();
	}

	bool MovementComponent::RefreshNavigationCellIndex()
	{
		if (_navMeshController == nullptr)
			return false;

		return _navMeshController->RefreshCellIndex();
	}

	void MovementComponent::OnInitialize()
	{
		_ownerTransform = GetOwner().GetTransform();
		GM_ASSERT_RETURN(_ownerTransform, "MovementComponent는 TransformComponent가 필요합니다.");
		_navMeshController = GetOwner().GetComponent<NavMeshControllerComponent>();
	}

	void MovementComponent::Move(const Vector3& desiredDelta)
	{
		if (_navMeshController && _navMeshController->IsMovementEnabled())
		{
			_navMeshController->Move(desiredDelta);
			return;
		}

		Translate(desiredDelta);
	}

	void MovementComponent::Translate(const Vector3& desiredDelta)
	{
		_ownerTransform->Translate(desiredDelta);
	}

	void MovementComponent::SetPosition(const Vector3& position)
	{
		_ownerTransform->SetPosition(position);
	}

	Vector3 MovementComponent::GetPosition() const
	{
		return _ownerTransform->GetPosition();
	}
}
