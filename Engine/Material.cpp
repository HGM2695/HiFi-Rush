#include "Material.h"
#include "IGraphicsCommandContext.h"
#include "PipelineState.h"
#include "Texture.h"

namespace gm
{
	std::shared_ptr<Material> Material::Create(const MaterialDesc& desc)
	{
		GM_ASSERT_RETURN_VAL(desc.pipelineState, nullptr, "Material PipelineState가 유효하지 않습니다.");
		return std::shared_ptr<Material>(new Material(desc));
	}

	Material::Material(const MaterialDesc& desc)
		: _pipelineState(desc.pipelineState), _textures(desc.textures)
	{
	}

	Material::~Material() = default;

	Texture* Material::GetTexture(uint32 slot) const
	{
		GM_ASSERT_RETURN_VAL(slot < MaxMaterialTextureSlots, nullptr, "Material Texture Slot이 범위를 벗어났습니다.");
		return _textures[slot].get();
	}

	void Material::SetPipelineState(const std::shared_ptr<PipelineState>& pipelineState)
	{
		GM_ASSERT_RETURN(pipelineState, "Material PipelineState가 유효하지 않습니다.");
		_pipelineState = pipelineState;
	}

	void Material::SetTexture(uint32 slot, const std::shared_ptr<Texture>& texture)
	{
		GM_ASSERT_RETURN(slot < MaxMaterialTextureSlots, "Material Texture Slot이 범위를 벗어났습니다.");
		_textures[slot] = texture;
	}

	void Material::Apply(IGraphicsCommandContext& commandContext) const
	{
		GM_ASSERT_RETURN(_pipelineState, "Material PipelineState가 유효하지 않습니다.");

		commandContext.SetPipelineState(*_pipelineState);
		for (uint32 slot = 0; slot < MaxMaterialTextureSlots; ++slot)
		{
			if (_textures[slot])
				commandContext.SetTexture(slot, *_textures[slot]);
		}
	}
}
