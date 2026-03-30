#include "PhysicsSystem2D.h"
#include "BoxCollider2D.h"
#include "CircleCollider2D.h"
#include "GameObject.h"
#include "Rigidbody2D.h"
#include "Scene.h"
#include "Transform.h"
#include "GMAssert.h"
#include <algorithm>
#include <cmath>

namespace gm
{
	namespace
	{
		constexpr float PHYSICS_GRAVITY = 2000.f;

		struct BoxBounds
		{
			math::Vector2 min;
			math::Vector2 max;
		};

		BoxBounds GetWorldBounds(const BoxCollider2D& collider, const GameObject& gameObject)
		{
			const math::Vector2 center = gameObject.GetTransform()->GetPosition() + collider.GetOffset();
			const math::Vector2 halfSize = collider.GetSize() * 0.5f;
			return { center - halfSize, center + halfSize };
		}

		math::Vector2 GetWorldCenter(const CircleCollider2D& collider, const GameObject& gameObject)
		{
			return gameObject.GetTransform()->GetPosition() + collider.GetOffset();
		}
	}

	void PhysicsSystem2D::Simulate(Scene& scene, float deltaTime)
	{
		float accumulatedTime = deltaTime;

		while (accumulatedTime > 0.f)
		{
			const float curTime = accumulatedTime > 0.016f ? 0.016f : accumulatedTime;
			accumulatedTime -= curTime;

			scene.ForEachAliveGameObject([this, &scene, curTime](GameObject& gameObject)
				{
					Rigidbody2D* rigidbody = gameObject.GetRigidbody2D();
					if (rigidbody == nullptr || rigidbody->IsKinematic())
						return;

					ApplyForces(*rigidbody, curTime);
					ApplyGravity(*rigidbody, curTime);
					ApplyLinearDamping(*rigidbody, curTime);
					ClampVelocity(*rigidbody);
					SimulateRigidbody(scene, gameObject, *rigidbody, curTime);
				});
		}

		scene.ForEachAliveGameObject([](GameObject& gameObject)
			{
				Rigidbody2D* rigidbody = gameObject.GetRigidbody2D();
				if (rigidbody == nullptr || rigidbody->IsKinematic())
					return;

				rigidbody->ClearForces();
			});
	}

	void PhysicsSystem2D::ApplyForces(Rigidbody2D& rigidbody, float deltaTime) const
	{
		GM_ASSERT_RETURN(rigidbody._mass > 0.f, "Rigidbody2D의 mass는 0보다 커야 합니다.");

		const math::Vector2 acceleration = rigidbody._accumulatedForce / rigidbody._mass;
		rigidbody._velocity += acceleration * deltaTime;
	}

	void PhysicsSystem2D::ApplyGravity(Rigidbody2D& rigidbody, float deltaTime) const
	{
		if (rigidbody._useGravity == false)
			return;

		rigidbody._velocity._y -= PHYSICS_GRAVITY * rigidbody._gravityScale * deltaTime;
	}

	void PhysicsSystem2D::ApplyLinearDamping(Rigidbody2D& rigidbody, float deltaTime) const
	{
		if (rigidbody._linearDamping <= 0.f)
			return;

		const float dampingFactor = std::exp(-rigidbody._linearDamping * deltaTime);
		rigidbody._velocity *= dampingFactor;
	}

	void PhysicsSystem2D::ClampVelocity(Rigidbody2D& rigidbody) const
	{
		if (rigidbody._maxSpeed <= 0.f)
			return;

		const float speed = rigidbody._velocity.Length();
		if (speed > rigidbody._maxSpeed)
			rigidbody._velocity = rigidbody._velocity.Normalized() * rigidbody._maxSpeed;
	}

	void PhysicsSystem2D::SimulateRigidbody(Scene& scene, GameObject& gameObject, Rigidbody2D& rigidbody, float deltaTime) const
	{
		rigidbody._isGrounded = false;

		const math::Vector2 movement = rigidbody._velocity * deltaTime;
		ResolveXAxis(scene, gameObject, rigidbody, movement);
		ResolveYAxis(scene, gameObject, rigidbody, movement);
	}

	void PhysicsSystem2D::ResolveXAxis(Scene& scene, GameObject& gameObject, Rigidbody2D& rigidbody, const math::Vector2& movement) const
	{
		Transform* transform = gameObject.GetTransform();
		transform->TranslateX(movement._x);

		scene.ForEachAliveGameObject([this, &gameObject, &rigidbody, transform](GameObject& otherObject)
		{
			if (&gameObject == &otherObject)
				return;

			Rigidbody2D* otherRigidbody = otherObject.GetRigidbody2D();
			if (otherRigidbody && otherRigidbody->IsKinematic() == false)
				return;

			for (const Collider2D* selfCollider : gameObject.GetColliders2D())
			{
				for (const Collider2D* otherCollider : otherObject.GetColliders2D())
				{
					const CollisionHit hit = CheckCollision(*selfCollider, gameObject, *otherCollider, otherObject);

					if (hit.isHit == false || std::fabs(hit.normal._x) < 0.5f)
						continue;

					transform->TranslateX(hit.normal._x * hit.penetrationDepth);
					rigidbody._velocity._x = 0.f;
				}
			}
		});
	}

	void PhysicsSystem2D::ResolveYAxis(Scene& scene, GameObject& gameObject, Rigidbody2D& rigidbody, const math::Vector2& movement) const
	{
		Transform* transform = gameObject.GetTransform();
		transform->TranslateY(movement._y);

		scene.ForEachAliveGameObject([this, &gameObject, &rigidbody, transform](GameObject& otherObject)
		{
			if (&gameObject == &otherObject)
				return;

			Rigidbody2D* otherRigidbody = otherObject.GetRigidbody2D();
			if (otherRigidbody && otherRigidbody->IsKinematic() == false)
				return;

			for (const Collider2D* selfCollider : gameObject.GetColliders2D())
			{
				for (const Collider2D* otherCollider : otherObject.GetColliders2D())
				{
					const CollisionHit hit = CheckCollision(*selfCollider, gameObject, *otherCollider, otherObject);

					if (hit.isHit == false || std::fabs(hit.normal._y) < 0.5f)
						continue;

					transform->TranslateY(hit.normal._y * hit.penetrationDepth);

					if (hit.normal._y > 0.f)
						rigidbody._isGrounded = true;

					rigidbody._velocity._y = 0.f;
				}
			}
		});
	}

	CollisionHit PhysicsSystem2D::CheckCollision(const Collider2D& lhs, const GameObject& lhsObject, const Collider2D& rhs, const GameObject& rhsObject) const
	{
		if (lhs.GetColliderType() == Collider2DType::Box && rhs.GetColliderType() == Collider2DType::Box)
			return CheckBoxCollision(static_cast<const BoxCollider2D&>(lhs), lhsObject, static_cast<const BoxCollider2D&>(rhs), rhsObject);

		if (lhs.GetColliderType() == Collider2DType::Circle && rhs.GetColliderType() == Collider2DType::Circle)
			return CheckCircleCollision(static_cast<const CircleCollider2D&>(lhs), lhsObject, static_cast<const CircleCollider2D&>(rhs), rhsObject);

		if (lhs.GetColliderType() == Collider2DType::Circle && rhs.GetColliderType() == Collider2DType::Box)
			return CheckCircleBoxCollision(static_cast<const CircleCollider2D&>(lhs), lhsObject, static_cast<const BoxCollider2D&>(rhs), rhsObject);

		if (lhs.GetColliderType() == Collider2DType::Box && rhs.GetColliderType() == Collider2DType::Circle)
			return CheckCircleBoxCollision(static_cast<const CircleCollider2D&>(rhs), rhsObject, static_cast<const BoxCollider2D&>(lhs), lhsObject);

		GM_ASSERT(false, "lhs, rhs 콜라이더 간의 타입 매칭이 존재하지 않습니다.");

		return CollisionHit();
	}

	CollisionHit PhysicsSystem2D::CheckBoxCollision(const BoxCollider2D& lhs, const GameObject& lhsObject, const BoxCollider2D& rhs, const GameObject& rhsObject) const
	{
		const BoxBounds lhsBounds = GetWorldBounds(lhs, lhsObject);
		const BoxBounds rhsBounds = GetWorldBounds(rhs, rhsObject);

		const float overlapX = (std::min)(lhsBounds.max._x, rhsBounds.max._x) - (std::max)(lhsBounds.min._x, rhsBounds.min._x);
		const float overlapY = (std::min)(lhsBounds.max._y, rhsBounds.max._y) - (std::max)(lhsBounds.min._y, rhsBounds.min._y);

		if (overlapX <= 0.f || overlapY <= 0.f)
			return {};

		CollisionHit hit{};
		hit.isHit = true;

		const math::Vector2 lhsCenter = (lhsBounds.min + lhsBounds.max) * 0.5f;
		const math::Vector2 rhsCenter = (rhsBounds.min + rhsBounds.max) * 0.5f;

		if (overlapX < overlapY)
		{
			hit.penetrationDepth = overlapX;
			hit.normal = lhsCenter._x < rhsCenter._x ? math::Vector2(-1.f, 0.f) : math::Vector2(1.f, 0.f);
			return hit;
		}

		hit.penetrationDepth = overlapY;
		hit.normal = lhsCenter._y < rhsCenter._y ? math::Vector2(0.f, -1.f) : math::Vector2(0.f, 1.f);
		return hit;
	}

	CollisionHit PhysicsSystem2D::CheckCircleCollision(const CircleCollider2D& lhs, const GameObject& lhsObject, const CircleCollider2D& rhs, const GameObject& rhsObject) const
	{
		const math::Vector2 lhsCenter = GetWorldCenter(lhs, lhsObject);
		const math::Vector2 rhsCenter = GetWorldCenter(rhs, rhsObject);
		const math::Vector2 delta = lhsCenter - rhsCenter;
		const float distanceSquared = delta.LengthSquared();
		const float radiusSum = lhs.GetRadius() + rhs.GetRadius();
		const float radiusSumSquared = radiusSum * radiusSum;

		if (distanceSquared >= radiusSumSquared)
			return {};

		CollisionHit hit{};
		hit.isHit = true;

		// 중점이 겹치는 경우
		if (distanceSquared <= 0.0001f)
		{
			hit.normal = math::Vector2(0.f, -1.f);
			hit.penetrationDepth = radiusSum;
			return hit;
		}

		const float distance = std::sqrt(distanceSquared);
		hit.normal = delta * (1.f / distance);
		hit.penetrationDepth = radiusSum - distance;
		return hit;
	}

	CollisionHit PhysicsSystem2D::CheckCircleBoxCollision(const CircleCollider2D& lhs, const GameObject& lhsObject, const BoxCollider2D& rhs, const GameObject& rhsObject) const
	{
		const math::Vector2 circleCenter = GetWorldCenter(lhs, lhsObject);
		const BoxBounds boxBounds = GetWorldBounds(rhs, rhsObject);
		const math::Vector2 closestPoint(std::clamp(circleCenter._x, boxBounds.min._x, boxBounds.max._x), std::clamp(circleCenter._y, boxBounds.min._y, boxBounds.max._y));
		const math::Vector2 delta = circleCenter - closestPoint;
		const float distanceSquared = delta.LengthSquared();
		const float radius = lhs.GetRadius();
		const float radiusSquared = radius * radius;

		if (distanceSquared >= radiusSquared)
			return {};

		CollisionHit hit{};
		hit.isHit = true;
		
		// 중점이 겹치는 경우
		if (distanceSquared <= 0.0001f)
		{
			const math::Vector2 boxCenter = (boxBounds.min + boxBounds.max) * 0.5f;
			const math::Vector2 centerDelta = circleCenter - boxCenter;
			const float dx = (rhs.GetSize()._x * 0.5f) - std::fabs(centerDelta._x);
			const float dy = (rhs.GetSize()._y * 0.5f) - std::fabs(centerDelta._y);

			if (dx < dy)
			{
				hit.normal = centerDelta._x < 0.f ? math::Vector2(-1.f, 0.f) : math::Vector2(1.f, 0.f);
				hit.penetrationDepth = radius + dx;
				return hit;
			}

			hit.normal = centerDelta._y < 0.f ? math::Vector2(0.f, -1.f) : math::Vector2(0.f, 1.f);
			hit.penetrationDepth = radius + dy;
			return hit;
		}

		const float distance = std::sqrt(distanceSquared);
		hit.normal = delta * (1.f / distance);
		hit.penetrationDepth = radius - distance;
		return hit;
	}
}
