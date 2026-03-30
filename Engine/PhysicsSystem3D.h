#pragma once

namespace gm
{
	class Scene;

	class PhysicsSystem3D
	{
	public:
		void Simulate(Scene& scene, float deltaTime);
	};
}
