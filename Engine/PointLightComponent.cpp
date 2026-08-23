#include "PointLightComponent.h"
#include "RenderTypes.h"

namespace gm
{
	void PointLightComponent::SetRange(float range)
	{
		GM_ASSERT_RETURN(range > 0.f, "Point Light Range는 0보다 커야 합니다.");
		_range = range;
	}

	void PointLightComponent::FillRenderItem(LightRenderItem& item) const
	{
		item.range = _range;
	}
}
