#pragma once

#include "GraphicsTypes.h"
#include "MathTypes.h"
#include "Rect.h"
#include "RenderTypes.h"
#include <memory>

namespace gm
{
	class Material;
	class Texture;
	struct SpriteFrame;

	class SpritePresenter
	{
	public:
		SpritePresenter();
		~SpritePresenter();

		void						EnsureDefaultMaterial();
		void						EnableEffectRendering();
		void						Submit(const Matrix& world) const;

		void						SetTexture(const std::shared_ptr<Texture>& texture, TextureSlot slot = TextureSlot::BaseColor);
		void						SetSamplerDesc(const SamplerDesc& desc, TextureSlot slot = TextureSlot::BaseColor);
		void						SetMaterial(const Material& material);
		void						SetSourceRect(const Rect& rect);
		void						SetSourceRect(const SpriteFrame& frame);
		void						DisableSourceRect();
		void						SetFacingMode(SpriteFacingMode facingMode);
		void						SetSortDepthOffset(float offset) { _sortDepthOffset = offset; }
		void						SetOpacity(float opacity);
		void						SetFillRatio(float ratio);
		void						SetRadialFill(const Vector2& center, float startAngle, float sweepAngle);
		void						SetDissolveTexture(const std::shared_ptr<Texture>& texture);
		void						SetDissolveThreshold(float threshold);
		void						DisableDissolve();
		void						SetEffectEmissive(const Color& color, float intensity);

		std::shared_ptr<Texture>	GetTexture(TextureSlot slot = TextureSlot::BaseColor) const;
		const SamplerDesc&			GetSamplerDesc(TextureSlot slot = TextureSlot::BaseColor) const;
		Material*					GetMaterial();
		const Material*				GetMaterial() const { return _material.get(); }

	private:
		Rect						CreateUVRect() const;
		void						UpdateSpriteConstantData();
		void						UpdateEffectConstantData();

	private:
		std::unique_ptr<Material>	_material = nullptr;
		bool						_useSourceRect = false;
		Rect						_sourceRect{};
		SpriteFacingMode			_facingMode = SpriteFacingMode::None;
		float						_sortDepthOffset = 0.f;
		float						_opacity = 1.f;
		float						_fillRatio = 1.f;
		uint32						_fillMode = 0;
		float						_radialStartAngle = 0.f;
		float						_radialSweepAngle = 0.f;
		Vector2						_radialCenter{ 0.5f, 0.5f };
		EffectMaterialConstantPS	_effectConstant{};
		bool						_isEffectRenderingEnabled = false;
	};
}
