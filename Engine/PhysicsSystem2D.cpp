#include "PhysicsSystem2D.h"
#include "BoxCollider2DComponent.h"
#include "CircleCollider2DComponent.h"
#include "GameObject.h"
#include "Rigidbody2DComponent.h"
#include "Scene.h"
#include "TransformComponent.h"
#include <algorithm>
#include <cmath>

namespace gm
{
	namespace
	{
		constexpr float PHYSICS_GRAVITY = 2000.f;

		struct BoxBounds
		{
			Vector2 min;
			Vector2 max;
		};

		BoxBounds GetWorldBounds(const BoxCollider2DComponent& collider, const GameObject& gameObject)
		{
			const Vector2 center = gameObject.GetTransform()->GetPosition2D() + collider.GetOffset();
			const Vector2 halfSize = collider.GetSize() * 0.5f;
			return { center - halfSize, center + halfSize };
		}

		Vector2 GetWorldCenter(const CircleCollider2DComponent& collider, const GameObject& gameObject)
		{
			return gameObject.GetTransform()->GetPosition2D() + collider.GetOffset();
		}
	}

	void PhysicsSystem2D::Simulate(Scene& scene, float deltaTime)
	{
		float accumulatedTime = deltaTime;

		while (accumulatedTime > 0.f)
		{
			const float curTime = accumulatedTime > 0.016f ? 0.016f : accumulatedTime;
			accumulatedTime -= curTime;

			scene.ForEachGameObject([this, &scene, curTime](GameObject& gameObject)
				{
					Rigidbody2DComponent* rigidbody = gameObject.GetRigidbody2D();
					if (rigidbody == nullptr || rigidbody->IsKinematic())
						return;

					ApplyForces(*rigidbody, curTime);
					ApplyGravity(*rigidbody, curTime);
					ApplyLinearDamping(*rigidbody, curTime);
					ClampVelocity(*rigidbody);
					SimulateRigidbody(scene, gameObject, *rigidbody, curTime);
				});
		}

		scene.ForEachGameObject([](GameObject& gameObject)
			{
				Rigidbody2DComponent* rigidbody = gameObject.GetRigidbody2D();
				if (rigidbody == nullptr || rigidbody->IsKinematic())
					return;

				rigidbody->ClearForces();
			});
	}

	void PhysicsSystem2D::ApplyForces(Rigidbody2DComponent& rigidbody, float deltaTime) const
	{
		GM_ASSERT_RETURN(rigidbody._mass > 0.f, "Rigidbody2D의 mass는 0보다 커야 합니다.");

		const Vector2 acceleration = rigidbody._accumulatedForce / rigidbody._mass;
		rigidbody._velocity += acceleration * deltaTime;
	}

	void PhysicsSystem2D::ApplyGravity(Rigidbody2DComponent& rigidbody, float deltaTime) const
	{
		if (rigidbody._useGravity == false)
			return;

		rigidbody._velocity.y -= PHYSICS_GRAVITY * rigidbody._gravityScale * deltaTime;
	}

	void PhysicsSystem2D::ApplyLinearDamping(Rigidbody2DComponent& rigidbody, float deltaTime) const
	{
		if (rigidbody._linearDamping <= 0.f)
			return;

		const float dampingFactor = std::exp(-rigidbody._linearDamping * deltaTime);
		rigidbody._velocity *= dampingFactor;
	}

	void PhysicsSystem2D::ClampVelocity(Rigidbody2DComponent& rigidbody) const
	{
		if (rigidbody._maxSpeed <= 0.f)
			return;

		const float speed = rigidbody._velocity.Length();
		if (speed > rigidbody._maxSpeed)
		{
			rigidbody._velocity.Normalize();
			rigidbody._velocity *= rigidbody._maxSpeed;
		}
	}

	void PhysicsSystem2D::SimulateRigidbody(Scene& scene, GameObject& gameObject, Rigidbody2DComponent& rigidbody, float deltaTime) const
	{
		rigidbody._isGrounded = false;

		const Vector2 movement = rigidbody._velocity * deltaTime;
		ResolveXAxis(scene, gameObject, rigidbody, movement);
		ResolveYAxis(scene, gameObject, rigidbody, movement);
	}

	void PhysicsSystem2D::ResolveXAxis(Scene& scene, GameObject& gameObject, Rigidbody2DComponent& rigidbody, const Vector2& movement) const
	{
		TransformComponent* transform = gameObject.GetTransform();
		transform->TranslateX(movement.x);

		scene.ForEachGameObject([this, &gameObject, &rigidbody, transform](GameObject& otherObject)
		{
			if (&gameObject == &otherObject)
				return;

			Rigidbody2DComponent* otherRigidbody = otherObject.GetRigidbody2D();
			if (otherRigidbody && otherRigidbody->IsKinematic() == false)
				return;

			for (const Collider2DComponent* selfCollider : gameObject.GetColliders2D())
			{
				for (const Collider2DComponent* otherCollider : otherObject.GetColliders2D())
				{
					const CollisionHit hit = CheckCollision(*selfCollider, gameObject, *otherCollider, otherObject);

					if (hit.isHit == false || std::fabs(hit.normal.x) < 0.5f)
						continue;

					transform->TranslateX(hit.normal.x * hit.penetrationDepth);
					rigidbody._velocity.x = 0.f;
				}
			}
		});
	}

	void PhysicsSystem2D::ResolveYAxis(Scene& scene, GameObject& gameObject, Rigidbody2DComponent& rigidbody, const Vector2& movement) const
	{
		TransformComponent* transform = gameObject.GetTransform();
		transform->TranslateY(movement.y);

		scene.ForEachGameObject([this, &gameObject, &rigidbody, transform](GameObject& otherObject)
		{
			if (&gameObject == &otherObject)
				return;

			Rigidbody2DComponent* otherRigidbody = otherObject.GetRigidbody2D();
			if (otherRigidbody && otherRigidbody->IsKinematic() == false)
				return;

			for (const Collider2DComponent* selfCollider : gameObject.GetColliders2D())
			{
				for (const Collider2DComponent* otherCollider : otherObject.GetColliders2D())
				{
					const CollisionHit hit = CheckCollision(*selfCollider, gameObject, *otherCollider, otherObject);

					if (hit.isHit == false || std::fabs(hit.normal.y) < 0.5f)
						continue;

					transform->TranslateY(hit.normal.y * hit.penetrationDepth);

					if (hit.normal.y > 0.f)
						rigidbody._isGrounded = true;

					rigidbody._velocity.y = 0.f;
				}
			}
		});
	}

	CollisionHit PhysicsSystem2D::CheckCollision(const Collider2DComponent& lhs, const GameObject& lhsObject, const Collider2DComponent& rhs, const GameObject& rhsObject) const
	{
		if (lhs.GetShapeType() == ColliderShape2DType::Box && rhs.GetShapeType() == ColliderShape2DType::Box)
			return CheckBoxCollision(static_cast<const BoxCollider2DComponent&>(lhs), lhsObject, static_cast<const BoxCollider2DComponent&>(rhs), rhsObject);

		if (lhs.GetShapeType() == ColliderShape2DType::Circle && rhs.GetShapeType() == ColliderShape2DType::Circle)
			return CheckCircleCollision(static_cast<const CircleCollider2DComponent&>(lhs), lhsObject, static_cast<const CircleCollider2DComponent&>(rhs), rhsObject);

		if (lhs.GetShapeType() == ColliderShape2DType::Circle && rhs.GetShapeType() == ColliderShape2DType::Box)
			return CheckCircleBoxCollision(static_cast<const CircleCollider2DComponent&>(lhs), lhsObject, static_cast<const BoxCollider2DComponent&>(rhs), rhsObject);

		if (lhs.GetShapeType() == ColliderShape2DType::Box && rhs.GetShapeType() == ColliderShape2DType::Circle)
			return CheckCircleBoxCollision(static_cast<const CircleCollider2DComponent&>(rhs), rhsObject, static_cast<const BoxCollider2DComponent&>(lhs), lhsObject);

		GM_ASSERT(false, "lhs, rhs 콜라이더 간의 타입 매칭이 존재하지 않습니다.");

		return CollisionHit();
	}

	CollisionHit PhysicsSystem2D::CheckBoxCollision(const BoxCollider2DComponent& lhs, const GameObject& lhsObject, const BoxCollider2DComponent& rhs, const GameObject& rhsObject) const
	{
		const BoxBounds lhsBounds = GetWorldBounds(lhs, lhsObject);
		const BoxBounds rhsBounds = GetWorldBounds(rhs, rhsObject);

		const float overlapX = (std::min)(lhsBounds.max.x, rhsBounds.max.x) - (std::max)(lhsBounds.min.x, rhsBounds.min.x);
		const float overlapY = (std::min)(lhsBounds.max.y, rhsBounds.max.y) - (std::max)(lhsBounds.min.y, rhsBounds.min.y);

		if (overlapX <= 0.f || overlapY <= 0.f)
			return {};

		CollisionHit hit{};
		hit.isHit = true;

		const Vector2 lhsCenter = (lhsBounds.min + lhsBounds.max) * 0.5f;
		const Vector2 rhsCenter = (rhsBounds.min + rhsBounds.max) * 0.5f;

		if (overlapX < overlapY)
		{
			hit.penetrationDepth = overlapX;
			hit.normal = lhsCenter.x < rhsCenter.x ? Vector2(-1.f, 0.f) : Vector2(1.f, 0.f);
			return hit;
		}

		hit.penetrationDepth = overlapY;
		hit.normal = lhsCenter.y < rhsCenter.y ? Vector2(0.f, -1.f) : Vector2(0.f, 1.f);
		return hit;
	}

	CollisionHit PhysicsSystem2D::CheckCircleCollision(const CircleCollider2DComponent& lhs, const GameObject& lhsObject, const CircleCollider2DComponent& rhs, const GameObject& rhsObject) const
	{
		const Vector2 lhsCenter = GetWorldCenter(lhs, lhsObject);
		const Vector2 rhsCenter = GetWorldCenter(rhs, rhsObject);
		const Vector2 delta = lhsCenter - rhsCenter;
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
			hit.normal = Vector2(0.f, -1.f);
			hit.penetrationDepth = radiusSum;
			return hit;
		}

		const float distance = std::sqrt(distanceSquared);
		hit.normal = delta * (1.f / distance);
		hit.penetrationDepth = radiusSum - distance;
		return hit;
	}

	CollisionHit PhysicsSystem2D::CheckCircleBoxCollision(const CircleCollider2DComponent& lhs, const GameObject& lhsObject, const BoxCollider2DComponent& rhs, const GameObject& rhsObject) const
	{
		const Vector2 circleCenter = GetWorldCenter(lhs, lhsObject);
		const BoxBounds boxBounds = GetWorldBounds(rhs, rhsObject);
		const Vector2 closestPoint(std::clamp(circleCenter.x, boxBounds.min.x, boxBounds.max.x), std::clamp(circleCenter.y, boxBounds.min.y, boxBounds.max.y));
		const Vector2 delta = circleCenter - closestPoint;
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
			const Vector2 boxCenter = (boxBounds.min + boxBounds.max) * 0.5f;
			const Vector2 centerDelta = circleCenter - boxCenter;
			const float dx = (rhs.GetSize().x * 0.5f) - std::fabs(centerDelta.x);
			const float dy = (rhs.GetSize().y * 0.5f) - std::fabs(centerDelta.y);

			if (dx < dy)
			{
				hit.normal = centerDelta.x < 0.f ? Vector2(-1.f, 0.f) : Vector2(1.f, 0.f);
				hit.penetrationDepth = radius + dx;
				return hit;
			}

			hit.normal = centerDelta.y < 0.f ? Vector2(0.f, -1.f) : Vector2(0.f, 1.f);
			hit.penetrationDepth = radius + dy;
			return hit;
		}

		const float distance = std::sqrt(distanceSquared);
		hit.normal = delta * (1.f / distance);
		hit.penetrationDepth = radius - distance;
		return hit;
	}
}
