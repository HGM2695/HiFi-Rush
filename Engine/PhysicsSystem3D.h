#pragma once

#include "CollisionTypes.h"
#include "GameObjectHandle.h"
#include <cstddef>
#include <vector>

namespace gm
{
	class Collider3DComponent;
	class GameObject;
	class Rigidbody3DComponent;
	class Scene;

	class PhysicsSystem3D
	{
	public:
		void						Simulate(Scene& scene, float deltaTime);
		const std::vector<ColliderPair>&	GetColliderPairs() const { return _colliderPairs; }

	private:
		struct ColliderPairElement
		{
			Collider3DComponent*	collider = nullptr;
			GameObjectHandle		owner{};

			bool operator==(const ColliderPairElement& rhs) const
			{
				return collider == rhs.collider && owner == rhs.owner;
			}
		};

		struct ColliderPairKey
		{
			ColliderPairElement	colliderA{};
			ColliderPairElement	colliderB{};

			bool operator==(const ColliderPairKey& rhs) const
			{
				return colliderA == rhs.colliderA && colliderB == rhs.colliderB;
			}
		};

		struct ColliderPairKeyHasher
		{
			size_t operator()(const ColliderPairKey& pair) const;
		};

		void	ApplyForces(Rigidbody3DComponent& rigidbody, float deltaTime) const;
		void	ApplyGravity(Rigidbody3DComponent& rigidbody, float deltaTime) const;
		void	ApplyLinearDamping(Rigidbody3DComponent& rigidbody, float deltaTime) const;
		void	ClampVelocity(Rigidbody3DComponent& rigidbody) const;
		void	IntegratePosition(GameObject& gameObject, Rigidbody3DComponent& rigidbody, float deltaTime) const;

		void	DetectCollisions(Scene& scene);
		bool	Intersects(const Collider3DComponent& lhs, const Collider3DComponent& rhs) const;
		void	UpdateColliderPairs(Scene& scene, std::vector<ColliderPairKey>&& detectedPairs);
		bool	IsPairAlive(const Scene& scene, const ColliderPairKey& pair) const;

		static ColliderPairKey MakePairKey(Collider3DComponent& lhs, Collider3DComponent& rhs);

		std::vector<ColliderPairKey>	_activePairs{};
		std::vector<ColliderPair>		_colliderPairs{};
		const Scene*					_trackedScene = nullptr;
	};
}
