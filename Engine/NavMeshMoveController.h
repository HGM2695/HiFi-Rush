#pragma once

#include "EngineCore.h"

namespace gm
{
	class TransformComponent;

	class NavMeshMoveController
	{
	public:
		void	SetEnabled(bool enabled) { _enabled = enabled; }
		bool	IsEnabled() const { return _enabled; }

		int32	GetCurrentCellIndex() const { return _currentCellIndex; }
		void	SetCurrentCellIndex(int32 cellIndex) { _currentCellIndex = cellIndex; }

		bool	ResetCellIndex(const Vector3& position);
		bool	Move(TransformComponent& transform, const Vector3& desiredDelta);

	private:
		int32	_currentCellIndex = -1;
		bool	_enabled = true;
	};
}
