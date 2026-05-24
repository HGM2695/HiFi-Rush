#pragma once

#include "Collider2DComponent.h"

namespace gm
{
	class CircleCollider2DComponent : public Collider2DComponent
	{
	public:
		CircleCollider2DComponent() : Collider2DComponent(Collider2DType::Circle) {}

		void	SetRadius(float radius) { _radius = radius; }
		float	GetRadius() const { return _radius; }

	protected:
		void	OnTick(float deltaTime) override;

	private:
		float	_radius = 50.f;
	};
}
