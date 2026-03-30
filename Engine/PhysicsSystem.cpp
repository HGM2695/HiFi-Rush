#include "PhysicsSystem.h"
#include "PhysicsSystem2D.h"
#include "PhysicsSystem3D.h"

namespace gm
{
	PhysicsSystem::PhysicsSystem()
		: _physicsSystem2D(std::make_unique<PhysicsSystem2D>()),
		_physicsSystem3D(std::make_unique<PhysicsSystem3D>())
	{
	}

	PhysicsSystem::~PhysicsSystem() = default;

	void PhysicsSystem::Simulate(Scene& scene, float deltaTime)
	{
		switch (_physicsMode)
		{
		case PhysicsMode::None:
			return;
		case PhysicsMode::Physics2D:
			_physicsSystem2D->Simulate(scene, deltaTime);
			return;
		case PhysicsMode::Physics3D:
			_physicsSystem3D->Simulate(scene, deltaTime);
			return;
		}
	}
}
