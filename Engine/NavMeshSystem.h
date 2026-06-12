#pragma once

#include "NavigationMesh.h"

#include <memory>

namespace gm
{
	class IDebugRenderer;

	class NavMeshSystem
	{
	public:
		void							SetActiveNavigationMesh(const std::shared_ptr<NavigationMesh>& navigationMesh);
		void							ClearActiveNavigationMesh();

		bool							HasActiveNavigationMesh() const { return _activeNavigationMesh != nullptr; }
		NavigationMoveResult			MoveOnActiveNavigationMesh(int32 currentCellIndex, const Vector3& currentPosition, const Vector3& desiredDelta) const;
		int32							FindCellIndex(const Vector3& position) const;
		std::shared_ptr<NavigationMesh> GetActiveNavigationMesh() const { return _activeNavigationMesh; }

#if GM_ENABLE_DEBUG_TOOLS
		void							DebugDraw(IDebugRenderer& debugRenderer) const;
#endif

	private:
		std::shared_ptr<NavigationMesh> _activeNavigationMesh;
	};
}
