#include "NavMeshControllerComponent.h"

#include "Application.h"
#include "GameObject.h"
#include "NavMeshSystem.h"
#include "PhysicsSystem.h"
#include "Rigidbody3DComponent.h"
#include "TransformComponent.h"

namespace gm
{
	namespace
	{
		constexpr float GroundContactEpsilon = 0.001f;
	}

	void NavMeshControllerComponent::OnInitialize()
	{
		_transform = GetOwner().GetTransform();
		GM_ASSERT_RETURN(_transform, "NavMeshControllerComponent는 TransformComponent가 필요합니다.");

		_rigidbody = GetOwner().GetRigidbody3D();
		GM_ASSERT_RETURN(_groundCollisionEnabled == false || _rigidbody, "NavMesh 바닥 충돌에는 Rigidbody3DComponent가 필요합니다.");
	}

	void NavMeshControllerComponent::OnTick(float)
	{
		CheckGroundCollision();
	}

	void NavMeshControllerComponent::SetMaxGroundSnapDownDistance(float distance)
	{
		GM_ASSERT_RETURN(distance >= 0.f, "지면 스냅 허용 거리는 0 이상이어야 합니다.");
		_maxGroundSnapDownDistance = distance;
	}

	bool NavMeshControllerComponent::RefreshCellIndex()
	{
		GM_ASSERT_RETURN_VAL(_transform, false, "NavMeshControllerComponent가 초기화되지 않았습니다.");

		NavMeshSystem& navMeshSystem = APPLICATION.GetPhysicsSystem().GetNavMeshSystem();
		if (navMeshSystem.HasActiveNavigationMesh() == false)
			return false;

		const int32 cellIndex = navMeshSystem.FindCellIndex(_transform->GetPosition());
		if (cellIndex < 0)
			return false;

		_currentCellIndex = cellIndex;
		return true;
	}

	bool NavMeshControllerComponent::Move(const Vector3& desiredDelta)
	{
		if (IsMovementEnabled() == false || _transform == nullptr)
			return false;

		NavMeshSystem& navMeshSystem = APPLICATION.GetPhysicsSystem().GetNavMeshSystem();
		if (navMeshSystem.HasActiveNavigationMesh() == false)
			return false;

		const Vector3 currentPosition = _transform->GetPosition();
		NavigationMoveResult result = navMeshSystem.MoveOnActiveNavigationMesh(_currentCellIndex, currentPosition, desiredDelta);
		if (result.isOnMesh == false)
			return false;

		_currentCellIndex = result.cellIndex;

		bool hasStartedFalling = false;
		const float groundHeight = result.position.y;
		if (_groundCollisionEnabled)
		{
			const float dropDistance = currentPosition.y - groundHeight;
			if (_groundState == GroundState::Grounded && dropDistance > _maxGroundSnapDownDistance)
			{
				result.position.y = currentPosition.y;
				_groundState = GroundState::Airborne;
				hasStartedFalling = _rigidbody == nullptr || _rigidbody->GetVelocity().y <= 0.f;
			}
			else if (_groundState != GroundState::Grounded)
			{
				result.position.y = currentPosition.y;
			}
		}

		_transform->SetPosition(result.position);

		if (hasStartedFalling)
		{
			NavigationGroundLostEvent event{};
			event.position = result.position;
			event.groundHeight = groundHeight;
			event.cellIndex = result.cellIndex;
			OnGroundLost.Publish(event);
		}

		return true;
	}

	void NavMeshControllerComponent::CheckGroundCollision()
	{
		const GroundState previousGroundState = _groundState;
		_groundState = GroundState::Airborne;
		if (_groundCollisionEnabled == false || _transform == nullptr || _rigidbody == nullptr || _rigidbody->IsEnabled() == false)
		{
			_groundState = GroundState::Uninitialized;
			return;
		}

		NavMeshSystem& navMeshSystem = APPLICATION.GetPhysicsSystem().GetNavMeshSystem();
		if (navMeshSystem.HasActiveNavigationMesh() == false)
		{
			_groundState = GroundState::Uninitialized;
			return;
		}

		Vector3 position = _transform->GetPosition();
		const NavigationGroundResult groundResult = navMeshSystem.QueryActiveNavigationGround(_currentCellIndex, position);
		if (groundResult.hasGround == false)
			return;

		_currentCellIndex = groundResult.cellIndex;
		const Vector3 velocity = _rigidbody->GetVelocity();
		const bool isPenetratingGround = position.y < groundResult.height - GroundContactEpsilon;
		const bool isDescendingOntoGround = velocity.y <= 0.f && position.y <= groundResult.height + GroundContactEpsilon;
		if (isPenetratingGround == false && isDescendingOntoGround == false)
			return;

		position.y = groundResult.height;
		_transform->SetPosition(position);

		Vector3 resolvedVelocity = velocity;
		resolvedVelocity.y = 0.f;
		_rigidbody->SetVelocity(resolvedVelocity);
		_groundState = GroundState::Grounded;

		if (previousGroundState == GroundState::Airborne)
		{
			NavigationGroundContactEvent event{};
			event.position = position;
			event.cellIndex = groundResult.cellIndex;
			OnGroundContact.Publish(event);
		}
	}
}
