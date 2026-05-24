#pragma once

#include "GraphicsTypes.h"
#include "MathTypes.h"
#include "Rect.h"
#include <memory>

namespace gm
{
	class Material;
	class PipelineState;
	class Sampler;
	class Texture;
	struct SpriteFrame;
	struct MaterialDesc;

	class SpritePresenter
	{
	public:
		SpritePresenter();
		~SpritePresenter();

		void						EnsureDefaultMaterial();
		void						Submit(const Matrix& world) const;

		void						SetTexture(const std::shared_ptr<Texture>& texture, MaterialSlot slot = MaterialSlot::BaseColor);
		void						SetSampler(const std::shared_ptr<Sampler>& sampler, MaterialSlot slot = MaterialSlot::BaseColor);
		void						SetPipelineState(const std::shared_ptr<PipelineState>& pipelineState);
		void						SetMaterial(const MaterialDesc& desc);
		void						SetMaterial(const Material& material);
		void						SetSourceRect(const Rect& rect);
		void						SetSourceRect(const SpriteFrame& frame);
		void						DisableSourceRect();

		std::shared_ptr<Texture>	GetTexture(MaterialSlot slot = MaterialSlot::BaseColor) const;
		std::shared_ptr<Sampler>	GetSampler(MaterialSlot slot = MaterialSlot::BaseColor) const;
		std::shared_ptr<PipelineState> GetPipelineState() const;
		Material*					GetMaterial() const { return _material.get(); }

	private:
		Rect						CreateUVRect() const;

	private:
		std::unique_ptr<Material>	_material;
		bool						_useSourceRect = false;
		Rect						_sourceRect{};
	};
}
