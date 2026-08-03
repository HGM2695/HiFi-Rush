#pragma once

namespace gm
{
	class GameObject;
	class Rigidbody3DComponent;
	class Scene;

	class PhysicsSystem3D
	{
	public:
		void	Simulate(Scene& scene, float deltaTime);

	private:
		void	ApplyForces(Rigidbody3DComponent& rigidbody, float deltaTime) const;
		void	ApplyGravity(Rigidbody3DComponent& rigidbody, float deltaTime) const;
		void	ApplyLinearDamping(Rigidbody3DComponent& rigidbody, float deltaTime) const;
		void	ClampVelocity(Rigidbody3DComponent& rigidbody) const;
		void	IntegratePosition(GameObject& gameObject, Rigidbody3DComponent& rigidbody, float deltaTime) const;
	};
}
