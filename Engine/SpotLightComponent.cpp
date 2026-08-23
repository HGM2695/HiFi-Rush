#include "SpotLightComponent.h"
#include "MathUtil.h"
#include "RenderTypes.h"

namespace gm
{
	SpotLightComponent::SpotLightComponent()
		: _innerConeRadian(Math::DegreesToRadians(20.f)), _outerConeRadian(Math::DegreesToRadians(30.f))
	{}

	void SpotLightComponent::SetRange(float range)
	{
		GM_ASSERT_RETURN(range > 0.f, "Spot Light Range는 0보다 커야 합니다.");
		_range = range;
	}

	void SpotLightComponent::SetConeRadians(float innerConeRadian, float outerConeRadian)
	{
		GM_ASSERT_RETURN(innerConeRadian >= 0.f && innerConeRadian < outerConeRadian && outerConeRadian <= Math::GM_PI * 0.5f, "Spot Light Cone Angle 범위가 유효하지 않습니다.");
		_innerConeRadian = innerConeRadian;
		_outerConeRadian = outerConeRadian;
	}

	void SpotLightComponent::FillRenderItem(LightRenderItem& item) const
	{
		item.range = _range;
		item.right = GetRight();
		item.innerConeRadian = _innerConeRadian;
		item.outerConeRadian = _outerConeRadian;
		item.spotConeFalloff = _coneFalloff;
		item.useSpotCookie = _isCookieEnabled;
	}
}
