#include "NavigationMesh.h"
#include "MathUtil.h"

#if GM_ENABLE_DEBUG_TOOLS
#include "IDebugRenderer.h"
#endif

namespace gm
{
	namespace
	{
		constexpr uint32 MaxMoveRetryCount = 16;
		constexpr float NavigationEdgeEpsilon = 0.0001f;
		constexpr float JumpCellHeightEpsilon = 0.01f;

		Plane CreatePlane(const std::array<Vector3, NavigationCell::PointCount>& points)
		{
			const Vector3 ab = points[1] - points[0];
			const Vector3 ac = points[2] - points[0];

			Vector3 normal = ab.Cross(ac);
			normal.Normalize();

			return Plane{ normal, -normal.Dot(points[0]) };
		}

#if GM_ENABLE_DEBUG_TOOLS
		Color GetNavigationCellDebugColor(NavigationCellType type)
		{
			switch (type)
			{
			case NavigationCellType::Normal:
				return Colors::Green;
			case NavigationCellType::Jump:
				return Colors::Blue;
			case NavigationCellType::Dead:
				return Colors::Red;
			case NavigationCellType::Dummy:
				return Colors::Gray;
			default:
				GM_ASSERT_RETURN_VAL(false, Colors::Magenta, "지원하지 않는 NavigationCellType입니다.");
			}
		}
#endif
	}

	/// Navigation Cell /////////////////////////////////////////////////////////////////////////////////
	bool NavigationCell::IsInside(const Vector3& position) const
	{
		for (uint32 edgeIndex = 0; edgeIndex < EdgeCount; ++edgeIndex)
		{
			if (IsOutsideEdge(position, edgeIndex))
				return false;
		}

		return true;
	}

	NavigationCellQueryResult NavigationCell::QueryPosition(const Vector3& position) const
	{
		NavigationCellQueryResult result{};
		result.slidePosition = position;

		for (uint32 edgeIndex = 0; edgeIndex < EdgeCount; ++edgeIndex)
		{
			if (IsOutsideEdge(position, edgeIndex) == false)
				continue;

			result.isInside = false;
			result.exitEdgeIndex = edgeIndex;
			result.neighborIndex = _neighborIndices[edgeIndex];
			return result;
		}

		return result;
	}

	NavigationCellMoveQueryResult NavigationMesh::QueryMovePosition(int32 cellIndex, const Vector3& targetPosition) const
	{
		const NavigationCell& cell = _cells[cellIndex];

		NavigationCellMoveQueryResult result{};
		result.state = NavigationCellMoveState::TargetPosition;
		result.targetPosition = targetPosition;
		result.slidePosition = targetPosition;

		for (uint32 edgeIndex = 0; edgeIndex < NavigationCell::EdgeCount; ++edgeIndex)
		{
			const Vector3& start = cell._points[edgeIndex];
			const Vector3& end = cell._points[(edgeIndex + 1) % NavigationCell::PointCount];

			// 시계방향 엣지 기준으로 바깥을 향하는 노멀을 계산
			const Vector3 edge = Math::ProjectOnXZPlane(end - start);

			Vector3 normal{ -edge.z, 0.f, edge.x };
			normal.Normalize();

			const Vector3 direction = Math::ProjectOnXZPlane(targetPosition - start);

			// dot이 elipson보다 작다면 해당 엣지 기준으로는 내부로 판정
			const float dot = normal.Dot(direction);
			if (dot <= NavigationEdgeEpsilon)
				continue;

			// 나간 엣지에 이동 가능한 이웃이 있으면 해당 셀에서 다시 검사
			const int32 neighborIndex = cell._neighborIndices[edgeIndex];
			if (CanEnterCell(neighborIndex, targetPosition))
			{
				result.state = NavigationCellMoveState::Recheck;
				result.neighborIndex = neighborIndex;
				return result;
			}

			// 막힌 엣지라면 targetPosition을 해당 엣지 위로 투영해 슬라이드
			Vector3 projectedPosition = start + direction - dot * normal;
			const Vector3 projectedDirection = Math::ProjectOnXZPlane(projectedPosition - start);

			// 투영점이 엣지 끝을 넘어가면 다음 엣지의 이웃을 따라 재검사
			if (projectedDirection.LengthSquared() > edge.LengthSquared())
			{
				const uint32 nextEdgeIndex = (edgeIndex + 1) % NavigationCell::EdgeCount;
				const int32 nextNeighborIndex = cell._neighborIndices[nextEdgeIndex];
				projectedPosition.y = targetPosition.y;
				if (CanEnterCell(nextNeighborIndex, projectedPosition))
				{
					result.state = NavigationCellMoveState::Recheck;
					result.targetPosition = projectedPosition;
					result.neighborIndex = nextNeighborIndex;
					return result;
				}

				result.state = NavigationCellMoveState::SlidePosition;
				result.slidePosition = end;
				result.slidePosition.y = targetPosition.y;
				return result;
			}

			// 투영점이 엣지 시작점보다 뒤에 있으면 이전 엣지의 이웃을 따라 재검사
			if (projectedDirection.Dot(edge) < 0.f)
			{
				const uint32 prevEdgeIndex = (edgeIndex + NavigationCell::EdgeCount - 1) % NavigationCell::EdgeCount;
				const int32 prevNeighborIndex = cell._neighborIndices[prevEdgeIndex];
				projectedPosition.y = targetPosition.y;
				if (CanEnterCell(prevNeighborIndex, projectedPosition))
				{
					result.state = NavigationCellMoveState::Recheck;
					result.targetPosition = projectedPosition;
					result.neighborIndex = prevNeighborIndex;
					return result;
				}

				result.state = NavigationCellMoveState::SlidePosition;
				result.slidePosition = start;
				result.slidePosition.y = targetPosition.y;
				return result;
			}

			// 투영점이 엣지 선분 안에 있으면 그 위치로 슬라이딩
			result.state = NavigationCellMoveState::SlidePosition;
			result.slidePosition = projectedPosition;
			result.slidePosition.y = targetPosition.y;
			return result;
		}

		return result;
	}

	bool NavigationMesh::CanEnterCell(int32 cellIndex, const Vector3& targetPosition) const
	{
		if (IsValidCellIndex(cellIndex) == false)
			return false;

		const NavigationCell& cell = _cells[cellIndex];
		switch (cell.GetType())
		{
		case NavigationCellType::Normal:
			return true;

		case NavigationCellType::Jump:
			return targetPosition.y + JumpCellHeightEpsilon >= cell.CalcHeight(targetPosition);

		case NavigationCellType::Dead:
		case NavigationCellType::Dummy:
		default:
			return false;
		}
	}

	bool NavigationCell::IsOutsideEdge(const Vector3& position, uint32 edgeIndex) const
	{
		const Vector3& start = _points[edgeIndex];
		const Vector3& end = _points[(edgeIndex + 1) % PointCount];

		const Vector3 edge = Math::ProjectOnXZPlane(end - start);

		Vector3 normal{ -edge.z, 0.f, edge.x };
		normal.Normalize();

		const Vector3 direction = Math::ProjectOnXZPlane(position - start);

		return normal.Dot(direction) > NavigationEdgeEpsilon;
	}

	float NavigationCell::CalcHeight(const Vector3& position) const
	{
		return _plane.CalcY(position.x, position.z, position.y);
	}

#if GM_ENABLE_DEBUG_TOOLS
	void NavigationCell::DebugDraw(IDebugRenderer& debugRenderer) const
	{
		const Color color = GetNavigationCellDebugColor(_type);
		debugRenderer.RequestDrawTriangle(_points[0], _points[1], _points[2], color);

		Vector3 point0 = _points[0];
		Vector3 point1 = _points[1];
		Vector3 point2 = _points[2];

		const Color lineColor = Colors::Magenta;
		debugRenderer.RequestDrawLine(point0, point1, lineColor);
		debugRenderer.RequestDrawLine(point1, point2, lineColor);
		debugRenderer.RequestDrawLine(point2, point0, lineColor);
	}
#endif

	std::shared_ptr<NavigationMesh> NavigationMesh::Create(const NavigationMeshDesc& desc)
	{
		GM_ASSERT_RETURN_VAL(desc.data.cells.empty() == false, nullptr, "NavigationMesh Cell 데이터가 비어 있습니다.");

		return std::shared_ptr<NavigationMesh>(new NavigationMesh(desc.data));
	}

	/// Navigation Mesh /////////////////////////////////////////////////////////////////////////////////
	NavigationMesh::NavigationMesh(const NavigationMeshData& data)
	{
		_cells.resize(data.cells.size());
		for (uint32 cellIndex = 0; cellIndex < data.cells.size(); ++cellIndex)
		{
			const NavigationCellData& source = data.cells[cellIndex];
			NavigationCell& cell = _cells[cellIndex];

			cell._points = source.points;
			cell._neighborIndices = source.neighborIndices;
			cell._neighborTypes = source.neighborTypes;
			cell._type = source.type;
			cell._plane = CreatePlane(cell._points);
		}
	}

	NavigationMoveResult NavigationMesh::Move(int32 currentCellIndex, const Vector3& currentPosition, const Vector3& desiredDelta) const
	{
		NavigationMoveResult result{};
		GM_ASSERT_RETURN_VAL(_cells.empty() == false, result, "NavigationMesh Cell이 비어 있습니다.");

		int32 cellIndex = currentCellIndex;
		if (IsValidCellIndex(cellIndex) == false || _cells[cellIndex].IsInside(currentPosition) == false)
			cellIndex = FindCellIndex(currentPosition);

		GM_ASSERT_RETURN_VAL(IsValidCellIndex(cellIndex), result, "currentPosition이 NavigationMesh 위에 있지 않습니다.");

		const int32 startCellIndex = cellIndex;
		Vector3 targetPosition = currentPosition + desiredDelta;
		for (uint32 retryCount = 0; retryCount < MaxMoveRetryCount; ++retryCount)
		{
			const NavigationCell& cell = _cells[cellIndex];
			const NavigationCellMoveQueryResult queryResult = QueryMovePosition(cellIndex, targetPosition);

			switch (queryResult.state)
			{
			case NavigationCellMoveState::TargetPosition:
				result.position = targetPosition;
				result.position.y = cell.CalcHeight(targetPosition);
				result.cellIndex = cellIndex;
				result.isOnMesh = true;
				return result;

			case NavigationCellMoveState::Recheck:
				cellIndex = queryResult.neighborIndex;
				targetPosition = queryResult.targetPosition;
				continue;

			case NavigationCellMoveState::SlidePosition:
				result.position = queryResult.slidePosition;
				result.position.y = cell.CalcHeight(queryResult.slidePosition);
				result.cellIndex = cellIndex;
				result.isOnMesh = true;
				result.isSliding = true;
				return result;

			default:
				GM_ASSERT_RETURN_VAL(false, result, "지원하지 않는 NavigationCellMoveState입니다.");
			}

			break;
		}

		GM_LOG(
			"NavigationMesh Move retry 횟수 초과로 fallback 발생. cellIndex=%d, current=(%.2f, %.2f, %.2f), target=(%.2f, %.2f, %.2f)",
			cellIndex,
			currentPosition.x, currentPosition.y, currentPosition.z,
			targetPosition.x, targetPosition.y, targetPosition.z
		);

		result.position = currentPosition;
		result.cellIndex = startCellIndex;
		result.isOnMesh = true;
		return result;
	}

	NavigationGroundResult NavigationMesh::QueryGround(int32 currentCellIndex, const Vector3& position) const
	{
		NavigationGroundResult result{};

		int32 cellIndex = currentCellIndex;
		if (IsValidCellIndex(cellIndex) == false || _cells[cellIndex].IsSearchable() == false)
			cellIndex = FindCellIndex(position);

		for (uint32 retryCount = 0; retryCount < MaxMoveRetryCount; ++retryCount)
		{
			if (IsValidCellIndex(cellIndex) == false)
				return result;

			const NavigationCell& cell = _cells[cellIndex];
			const NavigationCellQueryResult queryResult = cell.QueryPosition(position);
			if (queryResult.isInside)
			{
				result.height = cell.CalcHeight(position);
				result.cellIndex = cellIndex;
				result.hasGround = true;
				return result;
			}

			if (IsValidCellIndex(queryResult.neighborIndex) == false || _cells[queryResult.neighborIndex].IsSearchable() == false)
				return result;

			cellIndex = queryResult.neighborIndex;
		}

		return result;
	}

	int32 NavigationMesh::FindCellIndex(const Vector3& position) const
	{
		for (uint32 cellIndex = 0; cellIndex < _cells.size(); ++cellIndex)
		{
			if (_cells[cellIndex].IsSearchable() == false)
				continue;

			if (_cells[cellIndex].IsInside(position))
				return static_cast<int32>(cellIndex);
		}

		return -1;
	}

	const NavigationCell* NavigationMesh::GetCell(int32 cellIndex) const
	{
		if (IsValidCellIndex(cellIndex) == false)
			return nullptr;

		return &_cells[cellIndex];
	}

#if GM_ENABLE_DEBUG_TOOLS
	void NavigationMesh::DebugDraw(IDebugRenderer& debugRenderer) const
	{
		for (const NavigationCell& cell : _cells)
			cell.DebugDraw(debugRenderer);
	}
#endif

	bool NavigationMesh::IsValidCellIndex(int32 cellIndex) const
	{
		return 0 <= cellIndex && cellIndex < static_cast<int32>(_cells.size());
	}
}
