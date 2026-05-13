#include "CircleCollider2D.h"
#include "DebugRenderer.h"
#include "GameObject.h"
#include "Transform.h"

namespace gm
{
	void CircleCollider2D::OnLateUpdate()
	{
#ifdef _DEBUG
		const Vector2 worldCenter = GetOwner().GetTransform()->GetPosition2D() + GetOffset();
		const Color color = IsTrigger() ? Colors::Red : Colors::Green;
		debug::DebugRenderer::RequestDrawCircle(worldCenter, GetRadius(), color);
#endif
	}
}
