#pragma once

#include "Collider2DComponent.h"

namespace gm
{
	class BoxCollider2DComponent : public Collider2DComponent
	{
	public:
		BoxCollider2DComponent() : Collider2DComponent(ColliderShape2DType::Box) {}

		void					SetSize(const Vector2& size) { _size = size; }
		const Vector2&			GetSize() const { return _size; }

	protected:
		void					OnRender() override;

	private:
		Vector2					_size{ 100.f, 100.f };
	};
}
