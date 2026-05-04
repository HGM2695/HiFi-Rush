#include "BoxCollider2D.h"
#include "DebugRenderer.h"
#include "GameObject.h"
#include "Transform.h"

namespace gm
{
	void BoxCollider2D::OnLateUpdate()
	{
#ifdef _DEBUG
		const Vector2 worldCenter = GetOwner().GetTransform()->GetPosition() + GetOffset();
		const Color color = IsTrigger() ? Color::Red() : Color::Green();
		debug::DebugRenderer::RequestDrawRect(worldCenter, GetSize(), color);
#endif
	}
}
