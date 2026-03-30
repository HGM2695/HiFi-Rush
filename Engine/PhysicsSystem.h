#pragma once

#include <memory>

namespace gm
{
	class Scene;
	class PhysicsSystem2D;
	class PhysicsSystem3D;

	enum class PhysicsMode
	{
		None,
		Physics2D,
		Physics3D,
	};

	class PhysicsSystem
	{
	public:
		PhysicsSystem();
		~PhysicsSystem();

		void		SetPhysicsMode(PhysicsMode physicsMode) { _physicsMode = physicsMode; }
		PhysicsMode	GetPhysicsMode() const { return _physicsMode; }
		void		Simulate(Scene& scene, float deltaTime);

	private:
		std::unique_ptr<PhysicsSystem2D> _physicsSystem2D;
		std::unique_ptr<PhysicsSystem3D> _physicsSystem3D;
		PhysicsMode _physicsMode = PhysicsMode::None;
	};
}
