#include "NavMeshSystem.h"

#include "GMAssert.h"

#if GM_ENABLE_DEBUG_TOOLS
#include "IDebugRenderer.h"
#endif

namespace gm
{
	void NavMeshSystem::SetActiveNavigationMesh(const std::shared_ptr<NavigationMesh>& navigationMesh)
	{
		GM_ASSERT_RETURN(navigationMesh, "Active NavigationMesh로 설정하려는 NavigationMesh가 nullptr입니다.");

		_activeNavigationMesh = navigationMesh;
	}

	void NavMeshSystem::ClearActiveNavigationMesh()
	{
		_activeNavigationMesh = nullptr;
	}

	NavigationMoveResult NavMeshSystem::MoveOnActiveNavigationMesh(int32 currentCellIndex, const Vector3& currentPosition, const Vector3& desiredDelta) const
	{
		GM_ASSERT_RETURN_VAL(_activeNavigationMesh, NavigationMoveResult{}, "Active NavigationMesh가 설정되지 않았습니다.");

		return _activeNavigationMesh->Move(currentCellIndex, currentPosition, desiredDelta);
	}

	int32 NavMeshSystem::FindCellIndex(const Vector3& position) const
	{
		GM_ASSERT_RETURN_VAL(_activeNavigationMesh, -1, "Active NavigationMesh가 설정되지 않았습니다.");

		return _activeNavigationMesh->FindCellIndex(position);
	}

#if GM_ENABLE_DEBUG_TOOLS
	void NavMeshSystem::DebugDraw(IDebugRenderer& debugRenderer) const
	{
		if (_activeNavigationMesh == nullptr)
			return;

		_activeNavigationMesh->DebugDraw(debugRenderer);
	}
#endif
}
