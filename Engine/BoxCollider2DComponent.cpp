#include "BoxCollider2DComponent.h"
#include "Application.h"
#include "GameObject.h"
#include "TransformComponent.h"

#if GM_ENABLE_DEBUG_TOOLS
#include "IDebugRenderer.h"
#endif

namespace gm
{
	void BoxCollider2DComponent::OnTick(float deltaTime)
	{
#if GM_ENABLE_DEBUG_TOOLS
		const Vector2 worldCenter = GetOwner().GetTransform()->GetPosition2D() + GetOffset();
		const Color color = IsTrigger() ? Colors::Red : Colors::Green;
		APPLICATION.GetDebugRenderer().RequestDrawRect(worldCenter, GetSize(), color);
#endif
	}
}
