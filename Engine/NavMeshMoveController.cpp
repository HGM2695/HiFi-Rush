#include "NavMeshMoveController.h"

#include "Application.h"
#include "NavMeshSystem.h"
#include "PhysicsSystem.h"
#include "TransformComponent.h"

namespace gm
{
	bool NavMeshMoveController::ResetCellIndex(const Vector3& position)
	{
		NavMeshSystem& navMeshSystem = APPLICATION.GetPhysicsSystem().GetNavMeshSystem();
		if (navMeshSystem.HasActiveNavigationMesh() == false)
			return false;

		const int32 cellIndex = navMeshSystem.FindCellIndex(position);
		if (cellIndex < 0)
			return false;

		_currentCellIndex = cellIndex;
		return true;
	}

	bool NavMeshMoveController::Move(TransformComponent& transform, const Vector3& desiredDelta)
	{
		NavMeshSystem& navMeshSystem = APPLICATION.GetPhysicsSystem().GetNavMeshSystem();
		if (_enabled == false || navMeshSystem.HasActiveNavigationMesh() == false)
			return false;

		NavigationMoveResult result = navMeshSystem.MoveOnActiveNavigationMesh(
			_currentCellIndex,
			transform.GetPosition(),
			desiredDelta
		);

		if (result.isOnMesh == false)
			return false;

		_currentCellIndex = result.cellIndex;
		// 3D physcis 도입시 수정 필요
		transform.SetPosition(result.position);
		return true;
	}
}
