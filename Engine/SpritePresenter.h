#pragma once

#include "GraphicsTypes.h"
#include "MathTypes.h"
#include "Rect.h"
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
		void						Submit(const Matrix& world) const;

		void						SetTexture(const std::shared_ptr<Texture>& texture, TextureSlot slot = TextureSlot::BaseColor);
		void						SetSamplerDesc(const SamplerDesc& desc, TextureSlot slot = TextureSlot::BaseColor);
		void						SetMaterial(const Material& material);
		void						SetSourceRect(const Rect& rect);
		void						SetSourceRect(const SpriteFrame& frame);
		void						DisableSourceRect();

		std::shared_ptr<Texture>	GetTexture(TextureSlot slot = TextureSlot::BaseColor) const;
		const SamplerDesc&			GetSamplerDesc(TextureSlot slot = TextureSlot::BaseColor) const;
		Material*					GetMaterial();
		const Material*				GetMaterial() const { return _material.get(); }

	private:
		Rect						CreateUVRect() const;
		void						UpdateSpriteConstantData();

	private:
		std::unique_ptr<Material>	_material = nullptr;
		bool						_useSourceRect = false;
		Rect						_sourceRect{};
	};
}
