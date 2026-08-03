#pragma once

#include "NavigationTypes.h"
#include "Resource.h"

#include <vector>

namespace gm
{
	class IDebugRenderer;

	inline constexpr uint32 NavigationCellPointCount = 3;
	inline constexpr uint32 NavigationCellEdgeCount = 3;

	struct NavigationMeshDesc
	{
		NavigationMeshData data;
	};

	struct NavigationMoveResult
	{
		Vector3 position{};
		int32	cellIndex = -1;
		bool	isOnMesh = false;
		bool	isSliding = false;
	};

	struct NavigationGroundResult
	{
		float	height = 0.f;
		int32	cellIndex = -1;
		bool	hasGround = false;
	};

	struct NavigationCellQueryResult
	{
		bool	isInside = true;
		uint32	exitEdgeIndex = 0;
		int32	neighborIndex = -1;
		Vector3	slidePosition{};
	};

	enum class NavigationCellMoveState
	{
		TargetPosition,
		Recheck,
		SlidePosition,
	};

	struct NavigationCellMoveQueryResult
	{
		NavigationCellMoveState state = NavigationCellMoveState::TargetPosition;
		Vector3					targetPosition{};
		Vector3					slidePosition{};
		int32					neighborIndex = -1;
	};

	class NavigationCell
	{
	public:
		static constexpr uint32 PointCount = NavigationCellPointCount;
		static constexpr uint32 EdgeCount = NavigationCellEdgeCount;

		bool						IsInside(const Vector3& position) const;
		NavigationCellQueryResult	QueryPosition(const Vector3& position) const;
		NavigationCellMoveQueryResult QueryMovePosition(const Vector3& targetPosition) const;
		float						CalcHeight(const Vector3& position) const;
		bool						IsWalkable() const { return _type == NavigationCellType::Normal; }
		bool						IsSearchable() const { return _type != NavigationCellType::Dummy; }
#if GM_ENABLE_DEBUG_TOOLS
		void						DebugDraw(IDebugRenderer& debugRenderer) const;
#endif

		const Vector3&			GetPoint(uint32 index) const { return _points[index]; }
		int32					GetNeighborIndex(uint32 edgeIndex) const { return _neighborIndices[edgeIndex]; }
		NavigationCellType		GetType() const { return _type; }

	private:
		friend class NavigationMesh;

		bool					IsOutsideEdge(const Vector3& position, uint32 edgeIndex) const;

		std::array<Vector3, PointCount>				_points{};
		std::array<int32, EdgeCount>				_neighborIndices{ -1, -1, -1 };
		std::array<NavigationCellType, EdgeCount>	_neighborTypes{ NavigationCellType::Dummy, NavigationCellType::Dummy, NavigationCellType::Dummy };
		Plane										_plane{};
		NavigationCellType							_type = NavigationCellType::Normal;
	};

	class NavigationMesh : public Resource
	{
	public:
		static std::shared_ptr<NavigationMesh> Create(const NavigationMeshDesc& desc);

		static inline ResourceType Type = ResourceType::NavigationMesh;
		virtual ResourceType GetType() const override { return Type; }

		NavigationMoveResult	Move(int32 currentCellIndex, const Vector3& currentPosition, const Vector3& desiredDelta) const;
		NavigationGroundResult	QueryGround(int32 currentCellIndex, const Vector3& position) const;
		int32					FindCellIndex(const Vector3& position) const;
		const NavigationCell*	GetCell(int32 cellIndex) const;
		uint32					GetCellCount() const { return static_cast<uint32>(_cells.size()); }
#if GM_ENABLE_DEBUG_TOOLS
		void					DebugDraw(IDebugRenderer& debugRenderer) const;
#endif

	private:
		NavigationMesh() = default;
		explicit NavigationMesh(const NavigationMeshData& data);

		bool					IsValidCellIndex(int32 cellIndex) const;

	private:
		std::vector<NavigationCell> _cells;
	};
}
