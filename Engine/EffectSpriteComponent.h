#pragma once

#include "SpriteComponent.h"

namespace gm
{
	class EffectSpriteComponent final : public SpriteComponent
	{
	public:
		EffectSpriteComponent();
		~EffectSpriteComponent() override;

		void SetDissolveTexture(const std::shared_ptr<Texture>& texture) { GetPresenter().SetDissolveTexture(texture); }
		void SetDissolveThreshold(float threshold) { GetPresenter().SetDissolveThreshold(threshold); }
		void DisableDissolve() { GetPresenter().DisableDissolve(); }
		void SetEmissive(const Color& color, float intensity) { GetPresenter().SetEffectEmissive(color, intensity); }
		void SetSortDepthOffset(float offset) { GetPresenter().SetSortDepthOffset(offset); }
		void SetFillRatio(float ratio) { GetPresenter().SetFillRatio(ratio); }
		void SetRadialFill(const Vector2& center, float startAngle, float sweepAngle) { GetPresenter().SetRadialFill(center, startAngle, sweepAngle); }
		std::shared_ptr<Texture> GetTexture() const { return GetPresenter().GetTexture(); }
	};
}
