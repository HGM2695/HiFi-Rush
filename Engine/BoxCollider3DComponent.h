#pragma once

#include "Collider3DComponent.h"

namespace gm
{
	class BoxCollider3DComponent final : public Collider3DComponent
	{
	public:
		BoxCollider3DComponent() : Collider3DComponent(ColliderShape3DType::Box) {}

		void				SetSize(const Vector3& size);
		const Vector3&		GetSize() const { return _size; }

	private:
		Vector3				_size{ 1.f, 1.f, 1.f };
	};
}
