#pragma once

#include "LightComponent.h"

namespace gm
{
	class DirectionalLightComponent final : public LightComponent
	{
	public:
		LightType	GetLightType() const override { return LightType::Directional; }
		void		SetCastsShadow(bool castsShadow) { _castsShadow = castsShadow; }
		bool		CastsShadow() const { return _castsShadow; }

	private:
		void		FillRenderItem(LightRenderItem& item) const override;

	private:
		bool		_castsShadow = false;
	};
}
