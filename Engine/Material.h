#pragma once

#include "Resource.h"
#include <array>

namespace gm
{
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

	class Material final
	{
	public:
		Material() = default;
		Material(const MaterialDesc& desc);

		std::shared_ptr<PipelineState>	GetPipelineState() const { return _pipelineState; }
		std::shared_ptr<Texture>		GetTexture(uint32 slot) const;
		std::shared_ptr<Sampler>		GetSampler(uint32 slot) const;

		void							SetPipelineState(const std::shared_ptr<PipelineState>& pipelineState);
		void							SetTexture(uint32 slot, const std::shared_ptr<Texture>& texture);
		void							SetSampler(uint32 slot, const std::shared_ptr<Sampler>& sampler);

	private:
		std::shared_ptr<PipelineState>									_pipelineState = nullptr;
		std::array<std::shared_ptr<Texture>, MaxMaterialTextureSlots>	_textures{};
		std::array<std::shared_ptr<Sampler>, MaxMaterialSamplerSlots>	_samplers{};
	};
}
