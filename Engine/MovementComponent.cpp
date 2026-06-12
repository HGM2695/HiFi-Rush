#include "MovementComponent.h"

#include "GameObject.h"
#include "NavMeshMoveController.h"
#include "TransformComponent.h"

namespace gm
{
	MovementComponent::MovementComponent() = default;
	MovementComponent::~MovementComponent() = default;

	void MovementComponent::EnableNavigationMovement(bool enabled)
	{
		if (_navMeshMoveController == nullptr)
			_navMeshMoveController = std::make_unique<NavMeshMoveController>();

		_navMeshMoveController->SetEnabled(enabled);
	}

	bool MovementComponent::IsNavigationMovementEnabled() const
	{
		return _navMeshMoveController && _navMeshMoveController->IsEnabled();
	}

	bool MovementComponent::RefreshNavigationCellIndex()
	{
		if (_navMeshMoveController == nullptr)
			return false;

		return _navMeshMoveController->ResetCellIndex(_ownerTransform->GetPosition());
	}

	void MovementComponent::OnInitialize()
	{
		_ownerTransform = GetOwner().GetTransform();
		GM_ASSERT_RETURN(_ownerTransform, "MovementComponent는 TransformComponent가 필요합니다.");
	}

	void MovementComponent::Move(const Vector3& desiredDelta)
	{
		if (_navMeshMoveController && _navMeshMoveController->IsEnabled())
		{
			_navMeshMoveController->Move(*_ownerTransform, desiredDelta);
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
