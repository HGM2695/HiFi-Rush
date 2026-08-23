#pragma once

#include "LightComponent.h"

namespace gm
{
	class PointLightComponent final : public LightComponent
	{
	public:
		LightType	GetLightType() const override { return LightType::Point; }
		void		SetRange(float range);
		float		GetRange() const { return _range; }

	protected:
		void		FillRenderItem(LightRenderItem& item) const override;

	private:
		float		_range = 10.f;
	};
}
