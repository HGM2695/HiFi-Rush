#include "Material.h"
#include "PipelineState.h"
#include "Sampler.h"
#include "Texture.h"

namespace gm
{
	Material::Material(const MaterialDesc& desc) : _pipelineState(desc.pipelineState), _textures(desc.textures), _samplers(desc.samplers) {}

	std::shared_ptr<Texture> Material::GetTexture(MaterialSlot slot) const
	{
		GM_ASSERT_RETURN_VAL(ToMaterialSlotIndex(slot) < MaterialSlotCount, nullptr, "Material Texture Slot이 범위를 벗어났습니다.");
		return _textures[ToMaterialSlotIndex(slot)];
	}

	std::shared_ptr<Sampler> Material::GetSampler(MaterialSlot slot) const
	{
		GM_ASSERT_RETURN_VAL(ToMaterialSlotIndex(slot) < MaterialSlotCount, nullptr, "Material Sampler Slot이 범위를 벗어났습니다.");
		return _samplers[ToMaterialSlotIndex(slot)];
	}

	void Material::SetPipelineState(const std::shared_ptr<PipelineState>& pipelineState)
	{
		_pipelineState = pipelineState;
	}

	void Material::SetTexture(MaterialSlot slot, const std::shared_ptr<Texture>& texture)
	{
		GM_ASSERT_RETURN(ToMaterialSlotIndex(slot) < MaterialSlotCount, "Material Slot이 범위를 벗어났습니다.");
		_textures[ToMaterialSlotIndex(slot)] = texture;
	}

	void Material::SetSampler(MaterialSlot slot, const std::shared_ptr<Sampler>& sampler)
	{
		GM_ASSERT_RETURN(ToMaterialSlotIndex(slot) < MaterialSlotCount, "Material Slot이 범위를 벗어났습니다.");
		_samplers[ToMaterialSlotIndex(slot)] = sampler;
	}

}
