#include "Material.h"
#include "IGraphicsCommandContext.h"
#include "PipelineState.h"
#include "Sampler.h"
#include "Texture.h"

namespace gm
{
	Material::Material(const MaterialDesc& desc) : _pipelineState(desc.pipelineState), _textures(desc.textures), _samplers(desc.samplers) {}
	Material::~Material() = default;

	std::shared_ptr<Material> Material::Create(const MaterialDesc& desc)
	{
		GM_ASSERT_RETURN_VAL(desc.pipelineState, nullptr, "Material PipelineState가 유효하지 않습니다.");
		return std::shared_ptr<Material>(new Material(desc));
	}

	Texture* Material::GetTexture(uint32 slot) const
	{
		GM_ASSERT_RETURN_VAL(slot < MaxMaterialTextureSlots, nullptr, "Material Texture Slot이 범위를 벗어났습니다.");
		return _textures[slot].get();
	}

	Sampler* Material::GetSampler(uint32 slot) const
	{
		GM_ASSERT_RETURN_VAL(slot < MaxMaterialSamplerSlots, nullptr, "Material Sampler Slot이 범위를 벗어났습니다.");
		return _samplers[slot].get();
	}

	void Material::SetPipelineState(const std::shared_ptr<PipelineState>& pipelineState)
	{
		_pipelineState = pipelineState;
	}

	void Material::SetTexture(uint32 slot, const std::shared_ptr<Texture>& texture)
	{
		GM_ASSERT_RETURN(slot < MaxMaterialTextureSlots, "Material Texture Slot이 범위를 벗어났습니다.");
		_textures[slot] = texture;
	}

	void Material::SetSampler(uint32 slot, const std::shared_ptr<Sampler>& sampler)
	{
		GM_ASSERT_RETURN(slot < MaxMaterialSamplerSlots, "Material Sampler Slot이 범위를 벗어났습니다.");
		_samplers[slot] = sampler;
	}

	void Material::Apply(IGraphicsCommandContext& commandContext) const
	{
		commandContext.SetPipelineState(*_pipelineState);
		for (uint32 slot = 0; slot < MaxMaterialTextureSlots; ++slot)
		{
			if (_textures[slot])
				commandContext.SetTexture(slot, *_textures[slot]);
		}

		for (uint32 slot = 0; slot < MaxMaterialSamplerSlots; ++slot)
		{
			if (_samplers[slot])
				commandContext.SetSampler(slot, *_samplers[slot]);
		}
	}
}
