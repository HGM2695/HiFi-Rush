#pragma once

#include "Vector2.h"

namespace gm
{
	class Scene;
	class GameObject;
	class Collider2D;
	class Rigidbody2D;
	class BoxCollider2D;
	class CircleCollider2D;

	struct CollisionHit
	{
		bool			isHit = false;
		math::Vector2	normal{};
		float			penetrationDepth = 0.f;
	};

	class PhysicsSystem2D
	{
	public:
		void			Simulate(Scene& scene, float deltaTime);

	private:
		void			ApplyGravity(Rigidbody2D& rigidbody, float deltaTime) const;
		void			ApplyForces(Rigidbody2D& rigidbody, float deltaTime) const;
		void			ApplyLinearDamping(Rigidbody2D& rigidbody, float deltaTime) const;
		void			ClampVelocity(Rigidbody2D& rigidbody) const;
		void			SimulateRigidbody(Scene& scene, GameObject& gameObject, Rigidbody2D& rigidbody, float deltaTime) const;

		void			ResolveXAxis(Scene& scene, GameObject& gameObject, Rigidbody2D& rigidbody, const math::Vector2& movement) const;
		void			ResolveYAxis(Scene& scene, GameObject& gameObject, Rigidbody2D& rigidbody, const math::Vector2& movement) const;

		CollisionHit	CheckCollision(const Collider2D& lhs, const GameObject& lhsObject, const Collider2D& rhs, const GameObject& rhsObject) const;
		CollisionHit	CheckBoxCollision(const BoxCollider2D& lhs, const GameObject& lhsObject, const BoxCollider2D& rhs, const GameObject& rhsObject) const;
		CollisionHit	CheckCircleCollision(const CircleCollider2D& lhs, const GameObject& lhsObject, const CircleCollider2D& rhs, const GameObject& rhsObject) const;
		CollisionHit	CheckCircleBoxCollision(const CircleCollider2D& lhs, const GameObject& lhsObject, const BoxCollider2D& rhs, const GameObject& rhsObject) const;
	};
}
