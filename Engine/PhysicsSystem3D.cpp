#include "PhysicsSystem3D.h"
#include "BoxCollider3DComponent.h"
#include "Collider3DComponent.h"
#include "GameObject.h"
#include "HashUtil.h"
#include "Rigidbody3DComponent.h"
#include "Scene.h"
#include "SphereCollider3DComponent.h"
#include "TransformComponent.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <unordered_set>
#include <utility>
#include <vector>

namespace gm
{
	namespace
	{
		constexpr float PhysicsGravity = 9.81f;
		constexpr float MaxSimulationStep = 1.f / 60.f;
	}

	void PhysicsSystem3D::Simulate(Scene& scene, float deltaTime)
	{
		if (_trackedScene != &scene)
		{
			_activePairs.clear();
			_colliderPairs.clear();
			_trackedScene = &scene;
		}

		float remainingTime = deltaTime;
		while (remainingTime > 0.f)
		{
			const float simulationStep = (std::min)(remainingTime, MaxSimulationStep);
			remainingTime -= simulationStep;

			scene.ForEachGameObject([this, simulationStep](GameObject& gameObject)
			{
				Rigidbody3DComponent* rigidbody = gameObject.GetRigidbody3D();
				if (rigidbody == nullptr || rigidbody->IsEnabled() == false || rigidbody->IsKinematic())
					return;

				ApplyForces(*rigidbody, simulationStep);
				ApplyGravity(*rigidbody, simulationStep);
				ApplyLinearDamping(*rigidbody, simulationStep);
				ClampVelocity(*rigidbody);
				IntegratePosition(gameObject, *rigidbody, simulationStep);
			});
		}

		DetectCollisions(scene);

		scene.ForEachGameObject([](GameObject& gameObject)
		{
			Rigidbody3DComponent* rigidbody = gameObject.GetRigidbody3D();
			if (rigidbody)
				rigidbody->ClearForces();
		});
	}

	void PhysicsSystem3D::ApplyForces(Rigidbody3DComponent& rigidbody, float deltaTime) const
	{
		GM_ASSERT_RETURN(rigidbody._mass > 0.f, "Rigidbody3D의 mass는 0보다 커야 합니다.");

		const Vector3 acceleration = rigidbody._accumulatedForce / rigidbody._mass;
		rigidbody._velocity += acceleration * deltaTime;
	}

	void PhysicsSystem3D::ApplyGravity(Rigidbody3DComponent& rigidbody, float deltaTime) const
	{
		if (rigidbody._useGravity == false)
			return;

		rigidbody._velocity.y -= PhysicsGravity * rigidbody._gravityScale * deltaTime;
	}

	void PhysicsSystem3D::ApplyLinearDamping(Rigidbody3DComponent& rigidbody, float deltaTime) const
	{
		if (rigidbody._linearDamping <= 0.f)
			return;

		const float dampingFactor = std::exp(-rigidbody._linearDamping * deltaTime);
		rigidbody._velocity *= dampingFactor;
	}

	void PhysicsSystem3D::ClampVelocity(Rigidbody3DComponent& rigidbody) const
	{
		if (rigidbody._maxSpeed <= 0.f)
			return;

		const float speed = rigidbody._velocity.Length();
		if (speed <= rigidbody._maxSpeed)
			return;

		rigidbody._velocity.Normalize();
		rigidbody._velocity *= rigidbody._maxSpeed;
	}

	void PhysicsSystem3D::IntegratePosition(GameObject& gameObject, Rigidbody3DComponent& rigidbody, float deltaTime) const
	{
		TransformComponent* transform = gameObject.GetTransform();
		GM_ASSERT_RETURN(transform, "Rigidbody3D를 가진 GameObject에 TransformComponent가 존재하지 않습니다.");
		transform->Translate(rigidbody._velocity * deltaTime);
	}

	size_t PhysicsSystem3D::ColliderPairKeyHasher::operator()(const ColliderPairKey& pair) const
	{
		size_t seed = 0;
		HashValue(seed, pair.colliderA.collider);
		HashValue(seed, pair.colliderA.owner.index);
		HashValue(seed, pair.colliderA.owner.generation);
		HashValue(seed, pair.colliderB.collider);
		HashValue(seed, pair.colliderB.owner.index);
		HashValue(seed, pair.colliderB.owner.generation);
		return seed;
	}

	void PhysicsSystem3D::DetectCollisions(Scene& scene)
	{
		std::vector<Collider3DComponent*> colliders;
		scene.ForEachGameObject([&colliders](GameObject& gameObject)
		{
			for (Collider3DComponent* collider : gameObject.GetColliders3D())
			{
				if (collider == nullptr || collider->IsEnabled() == false)
					continue;

				collider->UpdateWorldShape();
				colliders.push_back(collider);
			}
		});

		std::vector<ColliderPairKey> detectedPairs;
		for (size_t lhsIndex = 0; lhsIndex < colliders.size(); ++lhsIndex)
		{
			Collider3DComponent& lhs = *colliders[lhsIndex];
			for (size_t rhsIndex = lhsIndex + 1; rhsIndex < colliders.size(); ++rhsIndex)
			{
				Collider3DComponent& rhs = *colliders[rhsIndex];
				if (&lhs.GetOwner() == &rhs.GetOwner())
					continue;

				if (ShouldCollide(lhs.GetCollisionFilter(), rhs.GetCollisionFilter()) == false)
					continue;

				if (Intersects(lhs, rhs) == false)
					continue;

				detectedPairs.push_back(MakePairKey(lhs, rhs));
			}
		}

		UpdateColliderPairs(scene, std::move(detectedPairs));
	}

	bool PhysicsSystem3D::Intersects(const Collider3DComponent& lhs, const Collider3DComponent& rhs) const
	{
		const ColliderShape3DType lhsType = lhs.GetShapeType();
		const ColliderShape3DType rhsType = rhs.GetShapeType();

		if (lhsType == ColliderShape3DType::Box && rhsType == ColliderShape3DType::Box)
		{
			const auto& lhsBox = static_cast<const BoxCollider3DComponent&>(lhs);
			const auto& rhsBox = static_cast<const BoxCollider3DComponent&>(rhs);
			return lhsBox.GetWorldShape().Intersects(rhsBox.GetWorldShape());
		}

		if (lhsType == ColliderShape3DType::Sphere && rhsType == ColliderShape3DType::Sphere)
		{
			const auto& lhsSphere = static_cast<const SphereCollider3DComponent&>(lhs);
			const auto& rhsSphere = static_cast<const SphereCollider3DComponent&>(rhs);
			return lhsSphere.GetWorldShape().Intersects(rhsSphere.GetWorldShape());
		}

		if (lhsType == ColliderShape3DType::Sphere && rhsType == ColliderShape3DType::Box)
		{
			const auto& lhsSphere = static_cast<const SphereCollider3DComponent&>(lhs);
			const auto& rhsBox = static_cast<const BoxCollider3DComponent&>(rhs);
			return lhsSphere.GetWorldShape().Intersects(rhsBox.GetWorldShape());
		}

		if (lhsType == ColliderShape3DType::Box && rhsType == ColliderShape3DType::Sphere)
		{
			const auto& lhsBox = static_cast<const BoxCollider3DComponent&>(lhs);
			const auto& rhsSphere = static_cast<const SphereCollider3DComponent&>(rhs);
			return rhsSphere.GetWorldShape().Intersects(lhsBox.GetWorldShape());
		}

		GM_ASSERT(false, "지원하지 않는 3D Collider Shape 조합입니다.");
		return false;
	}

	void PhysicsSystem3D::UpdateColliderPairs(Scene& scene, std::vector<ColliderPairKey>&& detectedPairs)
	{
		using ColliderPairSet = std::unordered_set<ColliderPairKey, ColliderPairKeyHasher>;

		const ColliderPairSet previousPairSet(_activePairs.begin(), _activePairs.end());
		const ColliderPairSet detectedPairSet(detectedPairs.begin(), detectedPairs.end());

		_colliderPairs.clear();
		_colliderPairs.reserve(detectedPairs.size() + _activePairs.size());

		for (const ColliderPairKey& pair : detectedPairs)
		{
			const ColliderPairState state = previousPairSet.contains(pair) ? ColliderPairState::Stay : ColliderPairState::Enter;
			_colliderPairs.push_back({ pair.colliderA.collider, pair.colliderB.collider, state });
		}

		for (const ColliderPairKey& pair : _activePairs)
		{
			if (detectedPairSet.contains(pair) || IsPairAlive(scene, pair) == false)
				continue;

			_colliderPairs.push_back({ pair.colliderA.collider, pair.colliderB.collider, ColliderPairState::Exit });
		}

		_activePairs = std::move(detectedPairs);
	}

	bool PhysicsSystem3D::IsPairAlive(const Scene& scene, const ColliderPairKey& pair) const
	{
		return scene.IsValid(pair.colliderA.owner) && scene.IsValid(pair.colliderB.owner);
	}

	PhysicsSystem3D::ColliderPairKey PhysicsSystem3D::MakePairKey(Collider3DComponent& lhs, Collider3DComponent& rhs)
	{
		ColliderPairElement lhsElement{ &lhs, lhs.GetOwner().GetHandle() };
		ColliderPairElement rhsElement{ &rhs, rhs.GetOwner().GetHandle() };

		if (std::less<Collider3DComponent*>{}(rhsElement.collider, lhsElement.collider))
			std::swap(lhsElement, rhsElement);

		return { lhsElement, rhsElement };
	}
}
