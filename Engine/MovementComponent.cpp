#include "MovementComponent.h"

#include "GameObject.h"
#include "NavMeshControllerComponent.h"
#include "TransformComponent.h"

namespace gm
{
	MovementComponent::MovementComponent() = default;
	MovementComponent::~MovementComponent() = default;

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
		if (_navMeshController && _navMeshController->IsEnabled())
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
