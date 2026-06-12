#pragma once

#include <memory>

namespace gm
{
	class Scene;
	class IDebugRenderer;
	class NavMeshSystem;
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
#if GM_ENABLE_DEBUG_TOOLS
		void		DebugRender(IDebugRenderer& debugRenderer) const;
#endif

		NavMeshSystem&			GetNavMeshSystem() { return *_navMeshSystem; }
		const NavMeshSystem&	GetNavMeshSystem() const { return *_navMeshSystem; }

	private:
		PhysicsMode							_physicsMode = PhysicsMode::None;
		std::unique_ptr<NavMeshSystem>		_navMeshSystem;
		std::unique_ptr<PhysicsSystem2D>	_physicsSystem2D;
		std::unique_ptr<PhysicsSystem3D>	_physicsSystem3D;
	};
}
