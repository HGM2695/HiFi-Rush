#include "DirectionalLightComponent.h"
#include "RenderTypes.h"

namespace gm
{
	void DirectionalLightComponent::FillRenderItem(LightRenderItem& item) const
	{
		item.castsShadow = _castsShadow;
	}
}
