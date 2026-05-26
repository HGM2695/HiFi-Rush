#include "Material.h"
#include "Resources.h"
#include "Shader.h"
#include "Texture.h"
#include "GraphicsUtils.h"

namespace gm
{
	namespace
	{
		void SetConstantDataInternal(Material::ConstantSlotsByShader& constantData, ShaderStage stage, uint32 slot, const void* data, uint32 size)
		{
			GM_ASSERT_RETURN(data, "Material Constant Data가 nullptr입니다.");
			GM_ASSERT_RETURN(size > 0, "Material Constant Data 크기가 0입니다.");
			GM_ASSERT_RETURN(slot < MaxConstantBufferSlots, "Material Constant Buffer Slot 범위를 벗어났습니다.");

			Material::ConstantSlot& slotData = constantData[ToShaderStageIndex(stage)][slot];
			slotData.bytes.assign(Align16(size), 0);
			std::memcpy(slotData.bytes.data(), data, size);
		}
	}

	Material::Material(const MaterialBuilder& builder)
		: _textures(builder._textures)
		, _samplerDescs(builder._samplerDescs)
		, _constantData(builder._constantData)
		, _vertexShader(builder._vertexShader)
		, _pixelShader(builder._pixelShader)
		, _topology(builder._topology)
		, _rasterizerDesc(builder._rasterizerDesc)
		, _depthStencilDesc(builder._depthStencilDesc)
		, _blendDesc(builder._blendDesc)
	{}

	std::shared_ptr<Texture> Material::GetTexture(TextureSlot slot) const
	{
		GM_ASSERT_RETURN_VAL(ToTexturelSlotIndex(slot) < TextureSlotCount, nullptr, "Material Texture Slot 범위를 벗어났습니다.");
		return _textures[ToTexturelSlotIndex(slot)];
	}

	const SamplerDesc& Material::GetSamplerDesc(TextureSlot slot) const
	{
		GM_ASSERT_RETURN_VAL(ToTexturelSlotIndex(slot) < TextureSlotCount, _samplerDescs[0], "Material Sampler Slot 범위를 벗어났습니다.");
		return _samplerDescs[ToTexturelSlotIndex(slot)];
	}

	const Material::ConstantSlot& Material::GetConstantSlot(ShaderStage stage, uint32 slot) const
	{
		GM_ASSERT_RETURN_VAL(slot < MaxConstantBufferSlots, _constantData[0][0], "Material Constant Buffer Slot 범위를 벗어났습니다.");
		return _constantData[ToShaderStageIndex(stage)][slot];
	}

	const Material::ConstantSlots& Material::GetConstantSlots(ShaderStage stage) const
	{
		return _constantData[ToShaderStageIndex(stage)];
	}

	void Material::SetTexture(TextureSlot slot, const std::shared_ptr<Texture>& texture)
	{
		GM_ASSERT_RETURN(ToTexturelSlotIndex(slot) < TextureSlotCount, "Material Texture Slot 범위를 벗어났습니다.");
		_textures[ToTexturelSlotIndex(slot)] = texture;
	}

	void Material::SetSamplerDesc(TextureSlot slot, const SamplerDesc& desc)
	{
		GM_ASSERT_RETURN(ToTexturelSlotIndex(slot) < TextureSlotCount, "Material Sampler Slot 범위를 벗어났습니다.");
		_samplerDescs[ToTexturelSlotIndex(slot)] = desc;
	}

	void Material::SetConstantData(ShaderStage stage, uint32 slot, const void* data, uint32 size)
	{
		SetConstantDataInternal(_constantData, stage, slot, data, size);
	}

	void Material::SetVertexShader(const std::shared_ptr<Shader>& shader)
	{
		_vertexShader = shader;
	}

	void Material::SetPixelShader(const std::shared_ptr<Shader>& shader)
	{
		_pixelShader = shader;
	}

	void Material::SetTopology(PrimitiveTopology topology)
	{
		_topology = topology;
	}

	void Material::SetRasterizerDesc(const RasterizerDesc& desc)
	{
		_rasterizerDesc = desc;
	}

	void Material::SetDepthStencilDesc(const DepthStencilDesc& desc)
	{
		_depthStencilDesc = desc;
	}

	void Material::SetBlendDesc(const BlendDesc& desc)
	{
		_blendDesc = desc;
	}

	/// Material Builder /////////////////////////////////////////////////////////////////////////////////////////////
	Material::MaterialBuilder::MaterialBuilder(Resources& resources) : _resources(resources) { }

	Material::MaterialBuilder& Material::MaterialBuilder::SetVertexShader(const std::wstring& key)
	{
		_vertexShader = _resources.Find<Shader>(key);
		GM_ASSERT(_vertexShader, "%ls VertexShader를 찾을 수 없습니다.", key.c_str());
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetPixelShader(const std::wstring& key)
	{
		_pixelShader = _resources.Find<Shader>(key);
		GM_ASSERT(_pixelShader, "%ls PixelShader를 찾을 수 없습니다.", key.c_str());
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetTexture(TextureSlot slot, const std::wstring& key)
	{
		GM_ASSERT_RETURN_VAL(ToTexturelSlotIndex(slot) < TextureSlotCount, *this, "Material Texture Slot 범위를 벗어났습니다.");
		_textures[ToTexturelSlotIndex(slot)] = _resources.Find<gm::Texture>(key);
		GM_ASSERT(_textures[ToTexturelSlotIndex(slot)], "%ls Texture를 찾을 수 없습니다.", key.c_str());
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetSampler(TextureSlot slot, const SamplerDesc& desc)
	{
		GM_ASSERT_RETURN_VAL(ToTexturelSlotIndex(slot) < TextureSlotCount, *this, "Material Sampler Slot 범위를 벗어났습니다.");
		_samplerDescs[ToTexturelSlotIndex(slot)] = desc;
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetTopology(PrimitiveTopology topology)
	{
		_topology = topology;
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetRasterizer(const RasterizerDesc& desc)
	{
		_rasterizerDesc = desc;
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetDepthStencil(const DepthStencilDesc& desc)
	{
		_depthStencilDesc = desc;
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetBlend(const BlendDesc& desc)
	{
		_blendDesc = desc;
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetFillMode(FillMode mode)
	{
		_rasterizerDesc.fillMode = mode;
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetCullMode(CullMode mode)
	{
		_rasterizerDesc.cullMode = mode;
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetFrontCounterClockwise(bool enable)
	{
		_rasterizerDesc.frontCounterClockwise = enable;
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetDepthClipEnable(bool enable)
	{
		_rasterizerDesc.depthClipEnable = enable;
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetDepthEnable(bool enable)
	{
		_depthStencilDesc.depthEnable = enable;
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetDepthWriteEnable(bool enable)
	{
		_depthStencilDesc.depthWriteEnable = enable;
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetDepthFunc(CompareFunc func)
	{
		_depthStencilDesc.depthFunc = func;
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetStencilEnable(bool enable)
	{
		_depthStencilDesc.stencilEnable = enable;
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetBlendEnable(bool enable)
	{
		_blendDesc.blendEnable = enable;
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetBlendFactor(BlendFactor srcBlend, BlendFactor destBlend)
	{
		_blendDesc.srcBlend = srcBlend;
		_blendDesc.destBlend = destBlend;
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetBlendOp(BlendOp op)
	{
		_blendDesc.blendOp = op;
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetSamplerFilter(TextureSlot slot, TextureFilter filter)
	{
		GM_ASSERT_RETURN_VAL(ToTexturelSlotIndex(slot) < TextureSlotCount, *this, "Material Sampler Slot 범위를 벗어났습니다.");
		_samplerDescs[ToTexturelSlotIndex(slot)].filter = filter;
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetSamplerAddressMode(TextureSlot slot, TextureAddressMode addressMode)
	{
		return SetSamplerAddressMode(slot, addressMode, addressMode, addressMode);
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetSamplerAddressMode(
		TextureSlot slot,
		TextureAddressMode addressU,
		TextureAddressMode addressV,
		TextureAddressMode addressW)
	{
		GM_ASSERT_RETURN_VAL(ToTexturelSlotIndex(slot) < TextureSlotCount, *this, "Material Sampler Slot 범위를 벗어났습니다.");

		SamplerDesc& desc = _samplerDescs[ToTexturelSlotIndex(slot)];
		desc.addressU = addressU;
		desc.addressV = addressV;
		desc.addressW = addressW;
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetConstantData(ShaderStage stage, uint32 slot, const void* data, uint32 size)
	{
		SetConstantDataInternal(_constantData, stage, slot, data, size);
		return *this;
	}

	Material Material::MaterialBuilder::Build() const
	{
		GM_ASSERT(_vertexShader, "Material 생성에 필요한 VertexShader가 없습니다.");
		GM_ASSERT(_pixelShader, "Material 생성에 필요한 PixelShader가 없습니다.");
		return Material(*this);
	}
}
