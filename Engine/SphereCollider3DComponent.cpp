#include "SphereCollider3DComponent.h"
#include "GameObject.h"
#include "TransformComponent.h"

namespace gm
{
	void SphereCollider3DComponent::SetRadius(float radius)
	{
		GM_ASSERT_RETURN(radius > 0.f, "Sphere Collider 반지름은 0보다 커야 합니다.");
		_radius = radius;
	}

	void SphereCollider3DComponent::UpdateWorldShape()
	{
		const BoundingSphere localShape(GetLocalCenter(), _radius);
		localShape.Transform(_worldShape, GetOwner().GetTransform()->GetWorldMatrix());
	}
}
