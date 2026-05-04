#pragma once

#include "Collider2D.h"

namespace gm
{
	class BoxCollider2D : public Collider2D
	{
	public:
		BoxCollider2D() : Collider2D(Collider2DType::Box) {}

		void					SetSize(const Vector2& size) { _size = size; }
		const Vector2&			GetSize() const { return _size; }

	protected:
		void					OnLateUpdate() override;

	private:
		Vector2					_size{ 100.f, 100.f };
	};
}
