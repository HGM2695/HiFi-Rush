#pragma once

#include "Types.h"

namespace gm
{
	using CollisionLayer = uint32;
	using CollisionMask = uint32;

	inline constexpr CollisionLayer InvalidCollisionLayer = 0;
	inline constexpr CollisionLayer DefaultCollisionLayer = 1u << 0;
	inline constexpr CollisionMask AllCollisionLayers = ~CollisionMask{};

	enum class ColliderShape3DType
	{
		Box,
		Sphere,
	};

	struct CollisionFilter
	{
		CollisionLayer	layer = DefaultCollisionLayer;
		CollisionMask	mask = AllCollisionLayers;
	};

	inline constexpr bool IsSingleCollisionLayer(CollisionLayer layer)
	{
		return layer != InvalidCollisionLayer && (layer & (layer - 1)) == 0;
	}

	inline constexpr bool ShouldCollide(const CollisionFilter& lhs, const CollisionFilter& rhs)
	{
		return (lhs.mask & rhs.layer) != 0 && (rhs.mask & lhs.layer) != 0;
	}
}
