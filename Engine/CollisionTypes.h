#pragma once

#include "Types.h"

namespace gm
{
	class Collider3DComponent;

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

	enum class ColliderPairState
	{
		Enter,
		Stay,
		Exit,
	};

	struct ColliderPair
	{
		Collider3DComponent*	colliderA = nullptr;
		Collider3DComponent*	colliderB = nullptr;
		ColliderPairState		state = ColliderPairState::Enter;
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
