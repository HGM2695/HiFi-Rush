#include "PhysicsSystem3D.h"
#include "BoxCollider3DComponent.h"
#include "Collider3DComponent.h"
#include "GameObject.h"
#include "HashUtil.h"
#include "MathUtil.h"
#include "Rigidbody3DComponent.h"
#include "Scene.h"
#include "SphereCollider3DComponent.h"
#include "TransformComponent.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <limits>
#include <unordered_set>
#include <utility>
#include <vector>

namespace gm
{
	namespace
	{
		constexpr float PhysicsGravity = 9.81f;
		constexpr float MaxSimulationStep = 1.f / 60.f;
		constexpr float CollisionEpsilon = 0.0001f;
	}

	void PhysicsSystem3D::Simulate(Scene& scene, float deltaTime)
	{
		if (_trackedScene != &scene)
		{
			_activePairs.clear();
			_collisionResults.clear();
			_trackedScene = &scene;
		}

		float remainingTime = deltaTime;
		bool hasSimulated = false;
		const std::vector<CollisionPairKey> frameStartPairs = _activePairs;
		std::vector<DetectedCollision> detectedDuringFrame;
		std::vector<DetectedCollision> finalDetectedCollisions;
		while (remainingTime > 0.f)
		{
			const float simulationStep = (std::min)(remainingTime, MaxSimulationStep);
			remainingTime -= simulationStep;
			hasSimulated = true;

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

			finalDetectedCollisions = DetectCollisions(scene);
			AccumulateDetectedCollisions(detectedDuringFrame, finalDetectedCollisions);
		}

		if (hasSimulated == false)
		{
			finalDetectedCollisions = DetectCollisions(scene);
			AccumulateDetectedCollisions(detectedDuringFrame, finalDetectedCollisions);
		}

		FinalizeCollisionFrame(scene, frameStartPairs, detectedDuringFrame, std::move(finalDetectedCollisions));

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

	size_t PhysicsSystem3D::CollisionPairKeyHasher::operator()(const CollisionPairKey& pair) const
	{
		size_t seed = 0;
		HashValue(seed, pair.elementA.collider);
		HashValue(seed, pair.elementA.owner.index);
		HashValue(seed, pair.elementA.owner.generation);
		HashValue(seed, pair.elementB.collider);
		HashValue(seed, pair.elementB.owner.index);
		HashValue(seed, pair.elementB.owner.generation);
		HashEnum(seed, pair.type);
		return seed;
	}

	std::vector<PhysicsSystem3D::DetectedCollision> PhysicsSystem3D::DetectCollisions(Scene& scene)
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

		std::vector<DetectedCollision> detectedCollisions;
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

				CollisionContact contact{};
				if (CheckCollision(lhs, rhs, contact) == false)
					continue;

				if (lhs.IsTrigger() == false && rhs.IsTrigger() == false)
					ResolveCollision(lhs, rhs, contact);

				CollisionPairKey pair = MakePairKey(lhs, rhs);
				if (pair.elementA.collider != &lhs)
				{
					std::swap(contact.selfPoint, contact.otherPoint);
					contact.normal = -contact.normal;
				}
				detectedCollisions.push_back({ pair, contact });
			}
		}

		return detectedCollisions;
	}

	bool PhysicsSystem3D::CheckCollision(const Collider3DComponent& lhs, const Collider3DComponent& rhs, CollisionContact& outContact) const
	{
		const ColliderShape3DType lhsType = lhs.GetShapeType();
		const ColliderShape3DType rhsType = rhs.GetShapeType();

		if (lhsType == ColliderShape3DType::Box && rhsType == ColliderShape3DType::Box)
		{
			const auto& lhsBox = static_cast<const BoxCollider3DComponent&>(lhs);
			const auto& rhsBox = static_cast<const BoxCollider3DComponent&>(rhs);
			return CheckBoxCollision(lhsBox, rhsBox, outContact);
		}

		if (lhsType == ColliderShape3DType::Sphere && rhsType == ColliderShape3DType::Sphere)
		{
			const auto& lhsSphere = static_cast<const SphereCollider3DComponent&>(lhs);
			const auto& rhsSphere = static_cast<const SphereCollider3DComponent&>(rhs);
			return CheckSphereCollision(lhsSphere, rhsSphere, outContact);
		}

		if (lhsType == ColliderShape3DType::Sphere && rhsType == ColliderShape3DType::Box)
		{
			const auto& lhsSphere = static_cast<const SphereCollider3DComponent&>(lhs);
			const auto& rhsBox = static_cast<const BoxCollider3DComponent&>(rhs);
			return CheckSphereBoxCollision(lhsSphere, rhsBox, outContact);
		}

		if (lhsType == ColliderShape3DType::Box && rhsType == ColliderShape3DType::Sphere)
		{
			const auto& lhsBox = static_cast<const BoxCollider3DComponent&>(lhs);
			const auto& rhsSphere = static_cast<const SphereCollider3DComponent&>(rhs);
			if (CheckSphereBoxCollision(rhsSphere, lhsBox, outContact) == false)
				return false;

			std::swap(outContact.selfPoint, outContact.otherPoint);
			outContact.normal = -outContact.normal;
			return true;
		}

		GM_ASSERT(false, "지원하지 않는 3D Collider Shape 조합입니다.");
		return false;
	}

	bool PhysicsSystem3D::CheckBoxCollision(const BoxCollider3DComponent& lhs, const BoxCollider3DComponent& rhs, CollisionContact& outContact) const
	{
		const BoundingOrientedBox& lhsShape = lhs.GetWorldShape();
		const BoundingOrientedBox& rhsShape = rhs.GetWorldShape();

		const Quaternion lhsRotation(lhsShape.Orientation.x, lhsShape.Orientation.y, lhsShape.Orientation.z, lhsShape.Orientation.w);
		const Quaternion rhsRotation(rhsShape.Orientation.x, rhsShape.Orientation.y, rhsShape.Orientation.z, rhsShape.Orientation.w);
		const std::array<Vector3, 3> lhsAxes =
		{
			Vector3::Transform(Vector3(1.f, 0.f, 0.f), lhsRotation),
			Vector3::Transform(Vector3(0.f, 1.f, 0.f), lhsRotation),
			Vector3::Transform(Vector3(0.f, 0.f, 1.f), lhsRotation),
		};
		const std::array<Vector3, 3> rhsAxes =
		{
			Vector3::Transform(Vector3(1.f, 0.f, 0.f), rhsRotation),
			Vector3::Transform(Vector3(0.f, 1.f, 0.f), rhsRotation),
			Vector3::Transform(Vector3(0.f, 0.f, 1.f), rhsRotation),
		};

		const Vector3 lhsCenter(lhsShape.Center.x, lhsShape.Center.y, lhsShape.Center.z);
		const Vector3 rhsCenter(rhsShape.Center.x, rhsShape.Center.y, rhsShape.Center.z);
		const Vector3 lhsExtents(lhsShape.Extents.x, lhsShape.Extents.y, lhsShape.Extents.z);
		const Vector3 rhsExtents(rhsShape.Extents.x, rhsShape.Extents.y, rhsShape.Extents.z);
		const std::array<float, 3> lhsExtentValues = { lhsExtents.x, lhsExtents.y, lhsExtents.z };
		const std::array<float, 3> rhsExtentValues = { rhsExtents.x, rhsExtents.y, rhsExtents.z };
		const Vector3 centerDelta = lhsCenter - rhsCenter;

		float minimumPenetration = std::numeric_limits<float>::max();
		Vector3 minimumAxis{};

		const auto testAxis = [&](Vector3 axis)
		{
			const float axisLengthSquared = axis.LengthSquared();
			if (axisLengthSquared <= CollisionEpsilon * CollisionEpsilon)
				return true;

			axis *= 1.f / std::sqrt(axisLengthSquared);
			const float lhsRadius =
				lhsExtents.x * std::fabs(axis.Dot(lhsAxes[0])) +
				lhsExtents.y * std::fabs(axis.Dot(lhsAxes[1])) +
				lhsExtents.z * std::fabs(axis.Dot(lhsAxes[2]));
			const float rhsRadius =
				rhsExtents.x * std::fabs(axis.Dot(rhsAxes[0])) +
				rhsExtents.y * std::fabs(axis.Dot(rhsAxes[1])) +
				rhsExtents.z * std::fabs(axis.Dot(rhsAxes[2]));
			const float penetration = lhsRadius + rhsRadius - std::fabs(centerDelta.Dot(axis));
			if (penetration <= 0.f)
				return false;

			if (penetration < minimumPenetration)
			{
				minimumPenetration = penetration;
				minimumAxis = centerDelta.Dot(axis) < 0.f ? -axis : axis;
			}

			return true;
		};

		for (const Vector3& axis : lhsAxes)
		{
			if (testAxis(axis) == false)
				return false;
		}

		for (const Vector3& axis : rhsAxes)
		{
			if (testAxis(axis) == false)
				return false;
		}

		for (const Vector3& lhsAxis : lhsAxes)
		{
			for (const Vector3& rhsAxis : rhsAxes)
			{
				if (testAxis(lhsAxis.Cross(rhsAxis)) == false)
					return false;
			}
		}

		outContact.normal = minimumAxis;
		outContact.penetrationDepth = minimumPenetration;

		const auto calculateSupportPoint = [](const Vector3& center, const std::array<Vector3, 3>& axes,
			const std::array<float, 3>& extents, const Vector3& direction)
		{
			Vector3 supportPoint = center;
			for (size_t axisIndex = 0; axisIndex < axes.size(); ++axisIndex)
			{
				const float projectedDirection = direction.Dot(axes[axisIndex]);
				if (std::fabs(projectedDirection) <= CollisionEpsilon)
					continue;

				supportPoint += axes[axisIndex] * (projectedDirection < 0.f ? -extents[axisIndex] : extents[axisIndex]);
			}

			return supportPoint;
		};

		const Vector3 lhsSupportPoint = calculateSupportPoint(lhsCenter, lhsAxes, lhsExtentValues, -minimumAxis);
		const Vector3 rhsSupportPoint = calculateSupportPoint(rhsCenter, rhsAxes, rhsExtentValues, minimumAxis);
		outContact.selfPoint = lhsSupportPoint;
		outContact.otherPoint = rhsSupportPoint;
		return true;
	}

	bool PhysicsSystem3D::CheckSphereCollision(const SphereCollider3DComponent& lhs, const SphereCollider3DComponent& rhs, CollisionContact& outContact) const
	{
		const BoundingSphere& lhsShape = lhs.GetWorldShape();
		const BoundingSphere& rhsShape = rhs.GetWorldShape();
		const Vector3 lhsCenter(lhsShape.Center.x, lhsShape.Center.y, lhsShape.Center.z);
		const Vector3 rhsCenter(rhsShape.Center.x, rhsShape.Center.y, rhsShape.Center.z);
		const Vector3 centerDelta = lhsCenter - rhsCenter;
		const float distanceSquared = centerDelta.LengthSquared();
		const float radiusSum = lhsShape.Radius + rhsShape.Radius;

		if (distanceSquared >= radiusSum * radiusSum)
			return false;

		if (distanceSquared <= CollisionEpsilon * CollisionEpsilon)
		{
			outContact.normal = Vector3(0.f, 1.f, 0.f);
			outContact.penetrationDepth = radiusSum;
			const Vector3 lhsSurfacePoint = lhsCenter - outContact.normal * lhsShape.Radius;
			const Vector3 rhsSurfacePoint = rhsCenter + outContact.normal * rhsShape.Radius;
			outContact.selfPoint = lhsSurfacePoint;
			outContact.otherPoint = rhsSurfacePoint;
			return true;
		}

		const float distance = std::sqrt(distanceSquared);
		outContact.normal = centerDelta * (1.f / distance);
		outContact.penetrationDepth = radiusSum - distance;
		const Vector3 lhsSurfacePoint = lhsCenter - outContact.normal * lhsShape.Radius;
		const Vector3 rhsSurfacePoint = rhsCenter + outContact.normal * rhsShape.Radius;
		outContact.selfPoint = lhsSurfacePoint;
		outContact.otherPoint = rhsSurfacePoint;
		return true;
	}

	bool PhysicsSystem3D::CheckSphereBoxCollision(const SphereCollider3DComponent& lhs, const BoxCollider3DComponent& rhs, CollisionContact& outContact) const
	{
		const BoundingSphere& sphere = lhs.GetWorldShape();
		const BoundingOrientedBox& box = rhs.GetWorldShape();
		const Quaternion boxRotation(box.Orientation.x, box.Orientation.y, box.Orientation.z, box.Orientation.w);
		const std::array<Vector3, 3> boxAxes =
		{
			Vector3::Transform(Vector3(1.f, 0.f, 0.f), boxRotation),
			Vector3::Transform(Vector3(0.f, 1.f, 0.f), boxRotation),
			Vector3::Transform(Vector3(0.f, 0.f, 1.f), boxRotation),
		};

		const Vector3 sphereCenter(sphere.Center.x, sphere.Center.y, sphere.Center.z);
		const Vector3 boxCenter(box.Center.x, box.Center.y, box.Center.z);
		const Vector3 boxExtents(box.Extents.x, box.Extents.y, box.Extents.z);
		const Vector3 centerDelta = sphereCenter - boxCenter;
		const std::array<float, 3> localCenter =
		{
			centerDelta.Dot(boxAxes[0]),
			centerDelta.Dot(boxAxes[1]),
			centerDelta.Dot(boxAxes[2]),
		};
		const std::array<float, 3> extents = { boxExtents.x, boxExtents.y, boxExtents.z };

		Vector3 closestPoint = boxCenter;
		for (size_t axisIndex = 0; axisIndex < boxAxes.size(); ++axisIndex)
			closestPoint += boxAxes[axisIndex] * std::clamp(localCenter[axisIndex], -extents[axisIndex], extents[axisIndex]);

		const Vector3 closestDelta = sphereCenter - closestPoint;
		const float distanceSquared = closestDelta.LengthSquared();
		if (distanceSquared >= sphere.Radius * sphere.Radius)
			return false;

		if (distanceSquared > CollisionEpsilon * CollisionEpsilon)
		{
			const float distance = std::sqrt(distanceSquared);
			outContact.normal = closestDelta * (1.f / distance);
			outContact.penetrationDepth = sphere.Radius - distance;
			outContact.selfPoint = sphereCenter - outContact.normal * sphere.Radius;
			outContact.otherPoint = closestPoint;
			return true;
		}

		size_t closestFaceIndex = 0;
		float closestFaceDistance = std::numeric_limits<float>::max();
		for (size_t axisIndex = 0; axisIndex < boxAxes.size(); ++axisIndex)
		{
			const float faceDistance = extents[axisIndex] - std::fabs(localCenter[axisIndex]);
			if (faceDistance < closestFaceDistance)
			{
				closestFaceIndex = axisIndex;
				closestFaceDistance = faceDistance;
			}
		}

		const float faceDirection = localCenter[closestFaceIndex] < 0.f ? -1.f : 1.f;
		outContact.normal = boxAxes[closestFaceIndex] * faceDirection;
		outContact.penetrationDepth = sphere.Radius + closestFaceDistance;
		outContact.selfPoint = sphereCenter + outContact.normal * sphere.Radius;
		outContact.otherPoint = sphereCenter + boxAxes[closestFaceIndex] *
			(faceDirection * extents[closestFaceIndex] - localCenter[closestFaceIndex]);
		return true;
	}

	void PhysicsSystem3D::ResolveCollision(Collider3DComponent& lhs, Collider3DComponent& rhs, const CollisionContact& contact) const
	{
		Rigidbody3DComponent* lhsRigidbody = lhs.GetOwner().GetRigidbody3D();
		Rigidbody3DComponent* rhsRigidbody = rhs.GetOwner().GetRigidbody3D();

		const bool isLhsDynamic = lhsRigidbody != nullptr && lhsRigidbody->IsEnabled() && lhsRigidbody->IsKinematic() == false;
		const bool isRhsDynamic = rhsRigidbody != nullptr && rhsRigidbody->IsEnabled() && rhsRigidbody->IsKinematic() == false;
		const float lhsInverseMass = isLhsDynamic ? 1.f / lhsRigidbody->_mass : 0.f;
		const float rhsInverseMass = isRhsDynamic ? 1.f / rhsRigidbody->_mass : 0.f;
		const float totalInverseMass = lhsInverseMass + rhsInverseMass;
		if (totalInverseMass <= 0.f)
			return;

		const float correctionDepth = (std::max)(contact.penetrationDepth - CollisionEpsilon, 0.f);
		if (correctionDepth > 0.f)
		{
			const Vector3 correction = contact.normal * correctionDepth;
			if (isLhsDynamic)
				lhs.GetOwner().GetTransform()->Translate(correction * (lhsInverseMass / totalInverseMass));
			if (isRhsDynamic)
				rhs.GetOwner().GetTransform()->Translate(-correction * (rhsInverseMass / totalInverseMass));

			if (isLhsDynamic)
				UpdateWorldShapes(lhs.GetOwner());
			if (isRhsDynamic)
				UpdateWorldShapes(rhs.GetOwner());
		}

		const Vector3 lhsVelocity = lhsRigidbody != nullptr && lhsRigidbody->IsEnabled() ? lhsRigidbody->_velocity : Vector3{};
		const Vector3 rhsVelocity = rhsRigidbody != nullptr && rhsRigidbody->IsEnabled() ? rhsRigidbody->_velocity : Vector3{};
		const float closingSpeed = (lhsVelocity - rhsVelocity).Dot(contact.normal);
		if (closingSpeed >= 0.f)
			return;

		const float impulseMagnitude = -closingSpeed / totalInverseMass;
		if (isLhsDynamic)
			lhsRigidbody->_velocity += contact.normal * (impulseMagnitude * lhsInverseMass);
		if (isRhsDynamic)
			rhsRigidbody->_velocity -= contact.normal * (impulseMagnitude * rhsInverseMass);
	}

	void PhysicsSystem3D::UpdateWorldShapes(GameObject& gameObject) const
	{
		for (Collider3DComponent* collider : gameObject.GetColliders3D())
		{
			if (collider != nullptr && collider->IsEnabled())
				collider->UpdateWorldShape();
		}
	}

	bool PhysicsSystem3D::Raycast(Scene& scene, const Vector3& origin, const Vector3& direction, float maxDistance, RaycastHit3D& outHit, const CollisionQueryFilter& filter) const
	{
		GM_ASSERT_RETURN_VAL(direction.LengthSquared() > CollisionEpsilon * CollisionEpsilon, false, "Raycast Direction은 0 벡터일 수 없습니다.");
		GM_ASSERT_RETURN_VAL(maxDistance > 0.f, false, "Raycast 최대 거리는 0보다 커야 합니다.");

		Vector3 normalizedDirection = direction;
		normalizedDirection.Normalize();
		float closestDistance = maxDistance;

		scene.ForEachGameObject([this, &origin, &normalizedDirection, &filter, &outHit, &closestDistance](GameObject& gameObject)
		{
			for (Collider3DComponent* collider : gameObject.GetColliders3D())
			{
				if (collider == nullptr || IsQueryCandidate(*collider, filter) == false)
					continue;

				collider->UpdateWorldShape();
				float distance = 0.f;
				bool isHit = false;

				switch (collider->GetShapeType())
				{
				case ColliderShape3DType::Box:
					isHit = Math::IntersectsRay(static_cast<BoxCollider3DComponent*>(collider)->GetWorldShape(), origin, normalizedDirection, distance);
					break;
				case ColliderShape3DType::Sphere:
					isHit = Math::IntersectsRay(static_cast<SphereCollider3DComponent*>(collider)->GetWorldShape(), origin, normalizedDirection, distance);
					break;
				}

				if (isHit == false || distance < 0.f || distance > closestDistance)
					continue;

				closestDistance = distance;
				outHit.collider = collider;
				outHit.distance = distance;
				outHit.point = origin + normalizedDirection * distance;
				outHit.normal = distance <= CollisionEpsilon
					? -normalizedDirection
					: CalculateRaycastNormal(*collider, outHit.point, normalizedDirection);
			}
		});

		return outHit.collider != nullptr;
	}

	std::vector<Collider3DComponent*> PhysicsSystem3D::OverlapBox(Scene& scene, const Vector3& center, const Vector3& size, const Quaternion& rotation, const CollisionQueryFilter& filter) const
	{
		if (size.x <= 0.f || size.y <= 0.f || size.z <= 0.f)
		{
			GM_ASSERT(false, "OverlapBox Size는 모든 축에서 0보다 커야 합니다.");
			return {};
		}

		Quaternion normalizedRotation = rotation;
		if (normalizedRotation.LengthSquared() <= CollisionEpsilon * CollisionEpsilon)
		{
			GM_ASSERT(false, "OverlapBox Rotation이 유효하지 않습니다.");
			return {};
		}
		normalizedRotation.Normalize();

		const BoundingOrientedBox queryShape(center, size * 0.5f, normalizedRotation);
		std::vector<Collider3DComponent*> overlaps;
		scene.ForEachGameObject([this, &queryShape, &filter, &overlaps](GameObject& gameObject)
		{
			for (Collider3DComponent* collider : gameObject.GetColliders3D())
			{
				if (collider == nullptr || IsQueryCandidate(*collider, filter) == false)
					continue;

				collider->UpdateWorldShape();
				bool isOverlapping = false;
				switch (collider->GetShapeType())
				{
				case ColliderShape3DType::Box:
					isOverlapping = queryShape.Intersects(static_cast<BoxCollider3DComponent*>(collider)->GetWorldShape());
					break;
				case ColliderShape3DType::Sphere:
					isOverlapping = queryShape.Intersects(static_cast<SphereCollider3DComponent*>(collider)->GetWorldShape());
					break;
				}

				if (isOverlapping)
					overlaps.push_back(collider);
			}
		});

		return overlaps;
	}

	std::vector<Collider3DComponent*> PhysicsSystem3D::OverlapSphere(Scene& scene, const Vector3& center, float radius, const CollisionQueryFilter& filter) const
	{
		if (radius <= 0.f)
		{
			GM_ASSERT(false, "OverlapSphere Radius는 0보다 커야 합니다.");
			return {};
		}

		const BoundingSphere queryShape(center, radius);
		std::vector<Collider3DComponent*> overlaps;
		scene.ForEachGameObject([this, &queryShape, &filter, &overlaps](GameObject& gameObject)
		{
			for (Collider3DComponent* collider : gameObject.GetColliders3D())
			{
				if (collider == nullptr || IsQueryCandidate(*collider, filter) == false)
					continue;

				collider->UpdateWorldShape();
				bool isOverlapping = false;
				switch (collider->GetShapeType())
				{
				case ColliderShape3DType::Box:
					isOverlapping = queryShape.Intersects(static_cast<BoxCollider3DComponent*>(collider)->GetWorldShape());
					break;
				case ColliderShape3DType::Sphere:
					isOverlapping = queryShape.Intersects(static_cast<SphereCollider3DComponent*>(collider)->GetWorldShape());
					break;
				}

				if (isOverlapping)
					overlaps.push_back(collider);
			}
		});

		return overlaps;
	}

	bool PhysicsSystem3D::IsQueryCandidate(const Collider3DComponent& collider, const CollisionQueryFilter& filter) const
	{
		if (collider.IsEnabled() == false)
			return false;
		if (filter.ignoredOwner != nullptr && &collider.GetOwner() == filter.ignoredOwner)
			return false;
		if (filter.includeTriggers == false && collider.IsTrigger())
			return false;

		return (filter.mask & collider.GetCollisionLayer()) != 0;
	}

	Vector3 PhysicsSystem3D::CalculateRaycastNormal(
		const Collider3DComponent& collider,
		const Vector3& point,
		const Vector3& rayDirection) const
	{
		if (collider.GetShapeType() == ColliderShape3DType::Sphere)
		{
			const BoundingSphere& sphere = static_cast<const SphereCollider3DComponent&>(collider).GetWorldShape();
			Vector3 normal = point - Vector3(sphere.Center.x, sphere.Center.y, sphere.Center.z);
			if (normal.LengthSquared() <= CollisionEpsilon * CollisionEpsilon)
				return -rayDirection;

			normal.Normalize();
			return normal;
		}

		const BoundingOrientedBox& box = static_cast<const BoxCollider3DComponent&>(collider).GetWorldShape();
		const Quaternion boxRotation(box.Orientation.x, box.Orientation.y, box.Orientation.z, box.Orientation.w);
		const std::array<Vector3, 3> boxAxes =
		{
			Vector3::Transform(Vector3(1.f, 0.f, 0.f), boxRotation),
			Vector3::Transform(Vector3(0.f, 1.f, 0.f), boxRotation),
			Vector3::Transform(Vector3(0.f, 0.f, 1.f), boxRotation),
		};
		const std::array<float, 3> extents = { box.Extents.x, box.Extents.y, box.Extents.z };
		const Vector3 center(box.Center.x, box.Center.y, box.Center.z);
		const Vector3 centerDelta = point - center;

		size_t closestFaceIndex = 0;
		float closestFaceDistance = std::numeric_limits<float>::max();
		float closestFaceCoordinate = 0.f;
		for (size_t axisIndex = 0; axisIndex < boxAxes.size(); ++axisIndex)
		{
			const float coordinate = centerDelta.Dot(boxAxes[axisIndex]);
			const float faceDistance = std::fabs(extents[axisIndex] - std::fabs(coordinate));
			if (faceDistance < closestFaceDistance)
			{
				closestFaceIndex = axisIndex;
				closestFaceDistance = faceDistance;
				closestFaceCoordinate = coordinate;
			}
		}

		return closestFaceCoordinate < 0.f ? -boxAxes[closestFaceIndex] : boxAxes[closestFaceIndex];
	}

	void PhysicsSystem3D::AccumulateDetectedCollisions(std::vector<DetectedCollision>& accumulatedCollisions, const std::vector<DetectedCollision>& detectedCollisions) const
	{
		for (const DetectedCollision& detectedCollision : detectedCollisions)
		{
			auto accumulatedIter = std::find_if(accumulatedCollisions.begin(), accumulatedCollisions.end(),
				[&detectedCollision](const DetectedCollision& accumulatedCollision)
				{
					return accumulatedCollision.pair == detectedCollision.pair;
				});

			if (accumulatedIter == accumulatedCollisions.end())
				accumulatedCollisions.push_back(detectedCollision);
			else
				accumulatedIter->contact = detectedCollision.contact;
		}
	}

	void PhysicsSystem3D::FinalizeCollisionFrame(Scene& scene, const std::vector<CollisionPairKey>& frameStartPairs, const std::vector<DetectedCollision>& detectedDuringFrame, std::vector<DetectedCollision>&& finalDetectedCollisions)
	{
		using CollisionPairSet = std::unordered_set<CollisionPairKey, CollisionPairKeyHasher>;

		std::vector<CollisionPairKey> finalPairs;
		finalPairs.reserve(finalDetectedCollisions.size());
		for (const DetectedCollision& detectedCollision : finalDetectedCollisions)
			finalPairs.push_back(detectedCollision.pair);

		const CollisionPairSet frameStartPairSet(frameStartPairs.begin(), frameStartPairs.end());
		const CollisionPairSet finalPairSet(finalPairs.begin(), finalPairs.end());

		_collisionResults.clear();
		_collisionResults.reserve(frameStartPairs.size() + finalPairs.size() + detectedDuringFrame.size() * 2);

		for (const CollisionPairKey& pair : frameStartPairs)
		{
			if (finalPairSet.contains(pair) || IsPairAlive(scene, pair) == false)
				continue;

			_collisionResults.push_back({ pair.elementA.collider, pair.elementB.collider, pair.type, CollisionState::Exit, {} });
		}

		for (const DetectedCollision& detectedCollision : detectedDuringFrame)
		{
			const CollisionPairKey& pair = detectedCollision.pair;
			if (frameStartPairSet.contains(pair) || finalPairSet.contains(pair))
				continue;

			_collisionResults.push_back({ pair.elementA.collider, pair.elementB.collider, pair.type, CollisionState::Enter, detectedCollision.contact });
			_collisionResults.push_back({ pair.elementA.collider, pair.elementB.collider, pair.type, CollisionState::Exit, {} });
		}

		for (const DetectedCollision& detectedCollision : finalDetectedCollisions)
		{
			const CollisionPairKey& pair = detectedCollision.pair;
			const CollisionState state = frameStartPairSet.contains(pair) ? CollisionState::Stay : CollisionState::Enter;
			_collisionResults.push_back({ pair.elementA.collider, pair.elementB.collider, pair.type, state, detectedCollision.contact });
		}

		_activePairs = std::move(finalPairs);
		DispatchCollisionEvents();
	}

	void PhysicsSystem3D::DispatchCollisionEvents()
	{
		for (const CollisionResult& result : _collisionResults)
		{
			if (result.elementA == nullptr || result.elementB == nullptr)
			{
				GM_ASSERT(false, "Collider Pair에 유효하지 않은 Collider가 포함되어 있습니다.");
				continue;
			}

			result.elementA->DispatchCollisionEvent(result.state, result.type, result.contact, *result.elementB);

			CollisionContact oppositeContact = result.contact;
			std::swap(oppositeContact.selfPoint, oppositeContact.otherPoint);
			oppositeContact.normal = -oppositeContact.normal;
			result.elementB->DispatchCollisionEvent(result.state, result.type, oppositeContact, *result.elementA);
		}
	}

	bool PhysicsSystem3D::IsPairAlive(const Scene& scene, const CollisionPairKey& pair) const
	{
		return scene.IsValid(pair.elementA.owner) && scene.IsValid(pair.elementB.owner);
	}

	PhysicsSystem3D::CollisionPairKey PhysicsSystem3D::MakePairKey(Collider3DComponent& lhs, Collider3DComponent& rhs)
	{
		CollisionPairElement lhsElement{ &lhs, lhs.GetOwner().GetHandle() };
		CollisionPairElement rhsElement{ &rhs, rhs.GetOwner().GetHandle() };

		if (std::less<Collider3DComponent*>{}(rhsElement.collider, lhsElement.collider))
			std::swap(lhsElement, rhsElement);

		const CollisionType type = lhs.IsTrigger() || rhs.IsTrigger() ? CollisionType::Trigger : CollisionType::Contact;

		return { lhsElement, rhsElement, type };
	}
}
