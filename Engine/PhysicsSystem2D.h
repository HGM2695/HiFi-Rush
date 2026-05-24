#pragma once

#include "EngineCore.h"

namespace gm
{
	class Scene;
	class GameObject;
	class Collider2DComponent;
	class Rigidbody2DComponent;
	class BoxCollider2DComponent;
	class CircleCollider2DComponent;

	struct CollisionHit
	{
		bool			isHit = false;
		Vector2			normal{};
		float			penetrationDepth = 0.f;
	};

	class PhysicsSystem2D
	{
	public:
		void			Simulate(Scene& scene, float deltaTime);

	private:
		void			ApplyGravity(Rigidbody2DComponent& rigidbody, float deltaTime) const;
		void			ApplyForces(Rigidbody2DComponent& rigidbody, float deltaTime) const;
		void			ApplyLinearDamping(Rigidbody2DComponent& rigidbody, float deltaTime) const;
		void			ClampVelocity(Rigidbody2DComponent& rigidbody) const;
		void			SimulateRigidbody(Scene& scene, GameObject& gameObject, Rigidbody2DComponent& rigidbody, float deltaTime) const;

		void			ResolveXAxis(Scene& scene, GameObject& gameObject, Rigidbody2DComponent& rigidbody, const Vector2& movement) const;
		void			ResolveYAxis(Scene& scene, GameObject& gameObject, Rigidbody2DComponent& rigidbody, const Vector2& movement) const;

		CollisionHit	CheckCollision(const Collider2DComponent& lhs, const GameObject& lhsObject, const Collider2DComponent& rhs, const GameObject& rhsObject) const;
		CollisionHit	CheckBoxCollision(const BoxCollider2DComponent& lhs, const GameObject& lhsObject, const BoxCollider2DComponent& rhs, const GameObject& rhsObject) const;
		CollisionHit	CheckCircleCollision(const CircleCollider2DComponent& lhs, const GameObject& lhsObject, const CircleCollider2DComponent& rhs, const GameObject& rhsObject) const;
		CollisionHit	CheckCircleBoxCollision(const CircleCollider2DComponent& lhs, const GameObject& lhsObject, const BoxCollider2DComponent& rhs, const GameObject& rhsObject) const;
	};
}
