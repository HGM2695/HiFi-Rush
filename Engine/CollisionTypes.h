#pragma once

#include "MathTypes.h"

namespace gm
{
	class Collider3DComponent;
	class GameObject;

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

	enum class CollisionType
	{
		Contact,
		Trigger,
	};

	enum class CollisionResponseMode
	{
		Full,
		Planar,
	};

	enum class CollisionState
	{
		Enter,
		Stay,
		Exit,
	};

	struct CollisionContact
	{
		Vector3	selfPoint{};
		Vector3	otherPoint{};
		Vector3	normal{};
		float	penetrationDepth = 0.f;
	};

	struct CollisionResult
	{
		Collider3DComponent*	elementA = nullptr;
		Collider3DComponent*	elementB = nullptr;
		CollisionType			type = CollisionType::Contact;
		CollisionState			state = CollisionState::Enter;
		CollisionContact		contact{};
	};

	struct CollisionQueryFilter
	{
		CollisionMask		mask = AllCollisionLayers;
		const GameObject*	ignoredOwner = nullptr;
		bool				includeTriggers = false;
	};

	struct RaycastHit3D
	{
		Collider3DComponent*	collider = nullptr;
		Vector3					point{};
		Vector3					normal{};
		float					distance = 0.f;
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
