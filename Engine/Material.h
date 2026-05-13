#pragma once

#include "Resource.h"
#include <array>

namespace gm
{
	class IGraphicsCommandContext;
	class PipelineState;
	class Sampler;
	class Texture;

	inline constexpr uint32 MaxMaterialTextureSlots = 8;
	inline constexpr uint32 MaxMaterialSamplerSlots = 8;

	struct MaterialDesc
	{
		std::shared_ptr<PipelineState> pipelineState;
		std::array<std::shared_ptr<Texture>, MaxMaterialTextureSlots> textures{};
		std::array<std::shared_ptr<Sampler>, MaxMaterialSamplerSlots> samplers{};
	};

	class Material : public Resource
	{
	public:
		static std::shared_ptr<Material> Create(const MaterialDesc& desc);
		virtual ~Material();

		static constexpr ResourceType Type = ResourceType::Material;
		virtual ResourceType	GetType() const override { return Type; }

		PipelineState*			GetPipelineState() const { return _pipelineState.get(); }
		Texture*				GetTexture(uint32 slot) const;
		Sampler*				GetSampler(uint32 slot) const;

		void					SetPipelineState(const std::shared_ptr<PipelineState>& pipelineState);
		void					SetTexture(uint32 slot, const std::shared_ptr<Texture>& texture);
		void					SetSampler(uint32 slot, const std::shared_ptr<Sampler>& sampler);
		void					Apply(IGraphicsCommandContext& commandContext) const;

	private:
		Material(const MaterialDesc& desc);

	private:
		std::shared_ptr<PipelineState>									_pipelineState;
		std::array<std::shared_ptr<Texture>, MaxMaterialTextureSlots>	_textures{};
		std::array<std::shared_ptr<Sampler>, MaxMaterialSamplerSlots>	_samplers{};
	};
}
