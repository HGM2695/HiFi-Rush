#include "BoxCollider3DComponent.h"

namespace gm
{
	void BoxCollider3DComponent::SetSize(const Vector3& size)
	{
		GM_ASSERT_RETURN(size.x > 0.f && size.y > 0.f && size.z > 0.f, "Box Collider 크기는 모든 축에서 0보다 커야 합니다.");
		_size = size;
	}
}
