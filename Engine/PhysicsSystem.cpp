#include "PhysicsSystem.h"
#include "NavMeshSystem.h"
#include "PhysicsSystem2D.h"
#include "PhysicsSystem3D.h"

#if GM_ENABLE_DEBUG_TOOLS
#include "IDebugRenderer.h"
#endif

namespace gm
{
	PhysicsSystem::PhysicsSystem()
		: _navMeshSystem(std::make_unique<NavMeshSystem>()),
		_physicsSystem2D(std::make_unique<PhysicsSystem2D>()),
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

#if GM_ENABLE_DEBUG_TOOLS
	void PhysicsSystem::DebugRender(IDebugRenderer& debugRenderer) const
	{
		_navMeshSystem->DebugDraw(debugRenderer);
	}
#endif
}
