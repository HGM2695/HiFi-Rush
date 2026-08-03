#include "PhysicsSystem3D.h"
#include "GameObject.h"
#include "Rigidbody3DComponent.h"
#include "Scene.h"
#include "TransformComponent.h"

#include <algorithm>
#include <cmath>

namespace gm
{
	namespace
	{
		constexpr float PhysicsGravity = 9.81f;
		constexpr float MaxSimulationStep = 1.f / 60.f;
	}

	void PhysicsSystem3D::Simulate(Scene& scene, float deltaTime)
	{
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
}
