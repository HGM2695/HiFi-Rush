#pragma once

#include "CollisionTypes.h"
#include "GameObjectHandle.h"
#include <cstddef>
#include <vector>

namespace gm
{
	class Collider3DComponent;
	class BoxCollider3DComponent;
	class SphereCollider3DComponent;
	class GameObject;
	class IDebugRenderer;
	class Rigidbody3DComponent;
	class Scene;

	class PhysicsSystem3D
	{
	public:
		void								Simulate(Scene& scene, float deltaTime);
		const std::vector<CollisionResult>&	GetCollisionResults() const { return _collisionResults; }
		bool								Raycast(Scene& scene, const Vector3& origin, const Vector3& direction, float maxDistance, RaycastHit3D& outHit, const CollisionQueryFilter& filter = {}) const;
		std::vector<Collider3DComponent*>	OverlapBox(Scene& scene, const Vector3& center, const Vector3& size, const Quaternion& rotation = Quaternion(0.f, 0.f, 0.f, 1.f), const CollisionQueryFilter& filter = {}) const;
		std::vector<Collider3DComponent*>	OverlapSphere(Scene& scene, const Vector3& center, float radius, const CollisionQueryFilter& filter = {}) const;
#if GM_ENABLE_DEBUG_TOOLS
		void								DebugDraw(Scene& scene, IDebugRenderer& debugRenderer) const;
#endif

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

		struct DetectedCollision
		{
			CollisionPairKey	pair{};
			CollisionContact	contact{};
		};

		void	ApplyForces(Rigidbody3DComponent& rigidbody, float deltaTime) const;
		void	ApplyGravity(Rigidbody3DComponent& rigidbody, float deltaTime) const;
		void	ApplyLinearDamping(Rigidbody3DComponent& rigidbody, float deltaTime) const;
		void	ClampVelocity(Rigidbody3DComponent& rigidbody) const;
		void	IntegratePosition(GameObject& gameObject, Rigidbody3DComponent& rigidbody, float deltaTime) const;

		std::vector<DetectedCollision>	DetectCollisions(Scene& scene);
		bool	CheckCollision(const Collider3DComponent& lhs, const Collider3DComponent& rhs, CollisionContact& outContact) const;
		bool	CheckBoxCollision(const BoxCollider3DComponent& lhs, const BoxCollider3DComponent& rhs, CollisionContact& outContact) const;
		bool	CheckSphereCollision(const SphereCollider3DComponent& lhs, const SphereCollider3DComponent& rhs, CollisionContact& outContact) const;
		bool	CheckSphereBoxCollision(const SphereCollider3DComponent& lhs, const BoxCollider3DComponent& rhs, CollisionContact& outContact) const;
		void	ResolveCollision(Collider3DComponent& lhs, Collider3DComponent& rhs, const CollisionContact& contact) const;
		void	UpdateWorldShapes(GameObject& gameObject) const;
		void	AccumulateDetectedCollisions(std::vector<DetectedCollision>& accumulatedCollisions, const std::vector<DetectedCollision>& detectedCollisions) const;
		void	FinalizeCollisionFrame(Scene& scene, const std::vector<CollisionPairKey>& frameStartPairs,
			const std::vector<DetectedCollision>& detectedDuringFrame, std::vector<DetectedCollision>&& finalDetectedCollisions);
		void	DispatchCollisionEvents();
		bool	IsPairAlive(const Scene& scene, const CollisionPairKey& pair) const;
		bool	IsQueryCandidate(const Collider3DComponent& collider, const CollisionQueryFilter& filter) const;
		Vector3	CalculateRaycastNormal(const Collider3DComponent& collider, const Vector3& point, const Vector3& rayDirection) const;

		static CollisionPairKey MakePairKey(Collider3DComponent& lhs, Collider3DComponent& rhs);

		std::vector<CollisionPairKey>	_activePairs{};
		std::vector<CollisionResult>	_collisionResults{};
		const Scene*					_trackedScene = nullptr;
	};
}
