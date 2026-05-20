#include "BoxCollider2D.h"
#include "Application.h"
#include "IDebugRenderer.h"
#include "GameObject.h"
#include "Transform.h"

namespace gm
{
	void BoxCollider2D::OnTick(float deltaTime)
	{
#ifdef _DEBUG
		const Vector2 worldCenter = GetOwner().GetTransform()->GetPosition2D() + GetOffset();
		const Color color = IsTrigger() ? Colors::Red : Colors::Green;
		APPLICATION.GetDebugRenderer().RequestDrawRect(worldCenter, GetSize(), color);
#endif
	}
}
