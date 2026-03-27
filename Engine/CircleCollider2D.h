#pragma once

#include "Collider2D.h"

namespace gm
{
	class CircleCollider2D : public Collider2D
	{
	public:
		CircleCollider2D()
			: Collider2D(Collider2DType::Circle)
		{
		}

		void	SetRadius(float radius) { _radius = radius; }
		float	GetRadius() const { return _radius; }

	protected:
		void OnLateUpdate() override;

	private:
		float _radius = 50.f;
	};
}
