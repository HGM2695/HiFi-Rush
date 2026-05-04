#include "CircleCollider2D.h"
#include "DebugRenderer.h"
#include "GameObject.h"
#include "Transform.h"

namespace gm
{
	void CircleCollider2D::OnLateUpdate()
	{
#ifdef _DEBUG
		const Vector2 worldCenter = GetOwner().GetTransform()->GetPosition() + GetOffset();
		const Color color = IsTrigger() ? Color::Red() : Color::Green();
		debug::DebugRenderer::RequestDrawCircle(worldCenter, GetRadius(), color);
#endif
	}
}
