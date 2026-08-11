#include "SphereCollider3DComponent.h"

namespace gm
{
	void SphereCollider3DComponent::SetRadius(float radius)
	{
		GM_ASSERT_RETURN(radius > 0.f, "Sphere Collider 반지름은 0보다 커야 합니다.");
		_radius = radius;
	}
}
