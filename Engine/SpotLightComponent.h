#pragma once

#include "LightComponent.h"

namespace gm
{
	class SpotLightComponent final : public LightComponent
	{
	public:
		SpotLightComponent();

		LightType	GetLightType() const override { return LightType::Spot; }
		void		SetRange(float range);
		float		GetRange() const { return _range; }
		void		SetConeRadians(float innerConeRadian, float outerConeRadian);
		float		GetInnerConeRadian() const { return _innerConeRadian; }
		float		GetOuterConeRadian() const { return _outerConeRadian; }
		void		SetConeFalloff(SpotConeFalloff coneFalloff) { _coneFalloff = coneFalloff; }
		SpotConeFalloff GetConeFalloff() const { return _coneFalloff; }
		void		SetCookieEnabled(bool isEnabled) { _isCookieEnabled = isEnabled; }
		bool		IsCookieEnabled() const { return _isCookieEnabled; }

	protected:
		void		FillRenderItem(LightRenderItem& item) const override;

	private:
		float		_range = 10.f;
		float		_innerConeRadian = 0.f;
		float		_outerConeRadian = 0.f;
		SpotConeFalloff _coneFalloff = SpotConeFalloff::Smooth;
		bool		_isCookieEnabled = false;
	};
}
