#include "CircleCollider2D.h"
#include "Application.h"
#include "IDebugRenderer.h"
#include "GameObject.h"
#include "Transform.h"

namespace gm
{
	void CircleCollider2D::OnTick(float deltaTime)
	{
#ifdef _DEBUG
		const Vector2 worldCenter = GetOwner().GetTransform()->GetPosition2D() + GetOffset();
		const Color color = IsTrigger() ? Colors::Red : Colors::Green;
		APPLICATION.GetDebugRenderer().RequestDrawCircle(worldCenter, GetRadius(), color);
#endif
	}
}
