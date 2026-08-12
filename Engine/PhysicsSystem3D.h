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
		const std::vector<CollisionResult>&	GetCollisionResults() const { return _collisionResults; }

	private:
		struct CollisionPairElement
		{
			Collider3DComponent*	collider = nullptr;
			GameObjectHandle		owner{};

			bool operator==(const CollisionPairElement& rhs) const
			{
				return collider == rhs.collider && owner == rhs.owner;
			}
		};

		struct CollisionPairKey
		{
			CollisionPairElement	elementA{};
			CollisionPairElement	elementB{};
			CollisionType			type = CollisionType::Contact;

			bool operator==(const CollisionPairKey& rhs) const
			{
				return elementA == rhs.elementA && elementB == rhs.elementB && type == rhs.type;
			}
		};

		struct CollisionPairKeyHasher
		{
			size_t operator()(const CollisionPairKey& pair) const;
		};

		void	ApplyForces(Rigidbody3DComponent& rigidbody, float deltaTime) const;
		void	ApplyGravity(Rigidbody3DComponent& rigidbody, float deltaTime) const;
		void	ApplyLinearDamping(Rigidbody3DComponent& rigidbody, float deltaTime) const;
		void	ClampVelocity(Rigidbody3DComponent& rigidbody) const;
		void	IntegratePosition(GameObject& gameObject, Rigidbody3DComponent& rigidbody, float deltaTime) const;

		void	DetectCollisions(Scene& scene);
		bool	Intersects(const Collider3DComponent& lhs, const Collider3DComponent& rhs) const;
		void	UpdateCollisionPairs(Scene& scene, std::vector<CollisionPairKey>&& detectedPairs);
		void	DispatchCollisionEvents();
		bool	IsPairAlive(const Scene& scene, const CollisionPairKey& pair) const;

		static CollisionPairKey MakePairKey(Collider3DComponent& lhs, Collider3DComponent& rhs);

		std::vector<CollisionPairKey>	_activePairs{};
		std::vector<CollisionResult>	_collisionResults{};
		const Scene*					_trackedScene = nullptr;
	};
}
