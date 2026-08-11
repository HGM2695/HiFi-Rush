#pragma once

#include "Collider3DComponent.h"

namespace gm
{
	class SphereCollider3DComponent final : public Collider3DComponent
	{
	public:
		SphereCollider3DComponent() : Collider3DComponent(ColliderShape3DType::Sphere) {}

		void	SetRadius(float radius);
		float	GetRadius() const { return _radius; }
		const BoundingSphere& GetWorldShape() const { return _worldShape; }

	private:
		void	UpdateWorldShape() override;

		float			_radius = 0.5f;
		BoundingSphere	_worldShape{};
	};
}
