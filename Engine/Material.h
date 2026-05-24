#pragma once

#include "Resource.h"
#include "GraphicsTypes.h"
#include <array>

namespace gm
{
	class PipelineState;
	class Sampler;
	class Texture;

	struct MaterialDesc
	{
		std::shared_ptr<PipelineState> pipelineState;
		std::array<std::shared_ptr<Texture>, MaterialSlotCount> textures{};
		std::array<std::shared_ptr<Sampler>, MaterialSlotCount> samplers{};
	};

	class Material final
	{
	public:
		Material() = default;
		Material(const MaterialDesc& desc);

		std::shared_ptr<PipelineState>	GetPipelineState() const { return _pipelineState; }
		std::shared_ptr<Texture>		GetTexture(MaterialSlot slot) const;
		std::shared_ptr<Sampler>		GetSampler(MaterialSlot slot) const;

		void							SetPipelineState(const std::shared_ptr<PipelineState>& pipelineState);
		void							SetTexture(MaterialSlot slot, const std::shared_ptr<Texture>& texture);
		void							SetSampler(MaterialSlot slot, const std::shared_ptr<Sampler>& sampler);

	private:
		std::shared_ptr<PipelineState>							_pipelineState = nullptr;
		std::array<std::shared_ptr<Texture>, MaterialSlotCount>	_textures{};
		std::array<std::shared_ptr<Sampler>, MaterialSlotCount>	_samplers{};
	};
}
