#include "BoxCollider3DComponent.h"
#include "GameObject.h"
#include "TransformComponent.h"

namespace gm
{
	void BoxCollider3DComponent::SetSize(const Vector3& size)
	{
		GM_ASSERT_RETURN(size.x > 0.f && size.y > 0.f && size.z > 0.f, "Box Collider 크기는 모든 축에서 0보다 커야 합니다.");
		_size = size;
	}

	void BoxCollider3DComponent::UpdateWorldShape()
	{
		const BoundingOrientedBox localShape(GetLocalCenter(), _size * 0.5f, GetLocalRotation());
		localShape.Transform(_worldShape, GetOwner().GetTransform()->GetWorldMatrix());
	}
}
