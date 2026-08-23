#include "Material.h"
#include "Resources.h"
#include "Shader.h"
#include "Texture.h"
#include "GraphicsUtils.h"
#include "HashUtil.h"

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

		void HashRasterizerDesc(size_t& seed, const RasterizerDesc& desc)
		{
			HashEnum(seed, desc.fillMode);
			HashEnum(seed, desc.cullMode);
			HashValue(seed, desc.frontCounterClockwise);
			HashValue(seed, desc.depthClipEnable);
		}

		void HashDepthStencilDesc(size_t& seed, const DepthStencilDesc& desc)
		{
			HashValue(seed, desc.depthEnable);
			HashValue(seed, desc.depthWriteEnable);
			HashEnum(seed, desc.depthFunc);
			HashValue(seed, desc.stencilEnable);
		}

		void HashBlendDesc(size_t& seed, const BlendDesc& desc)
		{
			HashValue(seed, desc.blendEnable);
			HashEnum(seed, desc.srcBlend);
			HashEnum(seed, desc.destBlend);
			HashEnum(seed, desc.blendOp);
		}

		void HashSamplerDesc(size_t& seed, const SamplerDesc& desc)
		{
			HashEnum(seed, desc.filter);
			HashEnum(seed, desc.addressU);
			HashEnum(seed, desc.addressV);
			HashEnum(seed, desc.addressW);
		}
	}

	Material::Material(const MaterialBuilder& builder)
		: _surfaceData(builder._surfaceData)
		, _colorData(builder._colorData)
		, _textures(builder._textures)
		, _samplerDescs(builder._samplerDescs)
		, _constantData(builder._constantData)
		, _vertexShader(builder._vertexShader)
		, _pixelShader(builder._pixelShader)
		, _topology(builder._topology)
		, _rasterizerDesc(builder._rasterizerDesc)
		, _depthStencilDesc(builder._depthStencilDesc)
		, _blendDesc(builder._blendDesc)
	{}

	void Material::SetColorData(const MaterialColorData& data)
	{
		GM_ASSERT_RETURN(data.mode < MaterialColorMode::Count, "지원하지 않는 Material Color Mode입니다.");
		GM_ASSERT_RETURN(data.blendRatio >= 0.f && data.blendRatio <= 1.f, "Material Color Blend Ratio는 0과 1 사이여야 합니다.");
		_colorData = data;
	}

	void Material::SetColorBlend(const Color& color, float ratio)
	{
		GM_ASSERT_RETURN(ratio >= 0.f && ratio <= 1.f, "Material Color Blend Ratio는 0과 1 사이여야 합니다.");
		_colorData.mode = MaterialColorMode::Blend;
		_colorData.blendColor = color;
		_colorData.blendRatio = ratio;
	}

	void Material::SetOpacityGradient(const Color& lowColor, const Color& highColor)
	{
		_colorData.mode = MaterialColorMode::OpacityGradient;
		_colorData.opacityLowColor = lowColor;
		_colorData.opacityHighColor = highColor;
	}

	void Material::SetSurfaceData(const MaterialSurfaceData& data)
	{
		SetShadingModel(data.shadingModel);
		SetSurfaceMode(data.surfaceMode);
		SetOutlineMode(data.outlineMode);
		SetEmissiveColor(data.emissiveColor);
		SetEmissiveIntensity(data.emissiveIntensity);
		SetAlphaCutoff(data.alphaCutoff);
	}

	void Material::SetShadingModel(ShadingModel shadingModel)
	{
		GM_ASSERT_RETURN(shadingModel < ShadingModel::Count, "지원하지 않는 Material Shading Model입니다.");
		_surfaceData.shadingModel = shadingModel;
	}

	void Material::SetSurfaceMode(SurfaceMode surfaceMode)
	{
		GM_ASSERT_RETURN(surfaceMode < SurfaceMode::Count, "지원하지 않는 Material Surface Mode입니다.");
		_surfaceData.surfaceMode = surfaceMode;
		ApplySurfaceModePipelineState(surfaceMode, _depthStencilDesc, _blendDesc);
	}

	void Material::SetOutlineMode(OutlineMode outlineMode)
	{
		GM_ASSERT_RETURN(outlineMode < OutlineMode::Count, "지원하지 않는 Material Outline Mode입니다.");
		_surfaceData.outlineMode = outlineMode;
	}

	void Material::SetEmissiveIntensity(float intensity)
	{
		GM_ASSERT_RETURN(intensity >= 0.f, "Material Emissive Intensity는 0 이상이어야 합니다.");
		_surfaceData.emissiveIntensity = intensity;
	}

	void Material::SetAlphaCutoff(float alphaCutoff)
	{
		GM_ASSERT_RETURN(alphaCutoff >= 0.f && alphaCutoff <= 1.f, "Material Alpha Cutoff은 0과 1 사이여야 합니다.");
		_surfaceData.alphaCutoff = alphaCutoff;
	}

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

	size_t Material::GetRenderStateHash() const
	{
		size_t seed = 0;
		HashEnum(seed, _surfaceData.shadingModel);
		HashEnum(seed, _surfaceData.surfaceMode);
		HashEnum(seed, _surfaceData.outlineMode);
		HashValue(seed, _surfaceData.emissiveColor.x);
		HashValue(seed, _surfaceData.emissiveColor.y);
		HashValue(seed, _surfaceData.emissiveColor.z);
		HashValue(seed, _surfaceData.emissiveColor.w);
		HashValue(seed, _surfaceData.emissiveIntensity);
		HashValue(seed, _surfaceData.alphaCutoff);
		HashEnum(seed, _colorData.mode);
		HashValue(seed, _colorData.blendColor.x);
		HashValue(seed, _colorData.blendColor.y);
		HashValue(seed, _colorData.blendColor.z);
		HashValue(seed, _colorData.blendColor.w);
		HashValue(seed, _colorData.opacityLowColor.x);
		HashValue(seed, _colorData.opacityLowColor.y);
		HashValue(seed, _colorData.opacityLowColor.z);
		HashValue(seed, _colorData.opacityLowColor.w);
		HashValue(seed, _colorData.opacityHighColor.x);
		HashValue(seed, _colorData.opacityHighColor.y);
		HashValue(seed, _colorData.opacityHighColor.z);
		HashValue(seed, _colorData.opacityHighColor.w);
		HashValue(seed, _colorData.colorMultiplier.x);
		HashValue(seed, _colorData.colorMultiplier.y);
		HashValue(seed, _colorData.colorMultiplier.z);
		HashValue(seed, _colorData.colorMultiplier.w);
		HashValue(seed, _colorData.blendRatio);
		HashValue(seed, _textureUVOffset.x);
		HashValue(seed, _textureUVOffset.y);
		HashValue(seed, _vertexShader.get());
		HashValue(seed, _pixelShader.get());
		HashEnum(seed, _topology);
		HashRasterizerDesc(seed, _rasterizerDesc);
		HashDepthStencilDesc(seed, _depthStencilDesc);
		HashBlendDesc(seed, _blendDesc);

		for (uint32 textureIndex = 0; textureIndex < TextureSlotCount; ++textureIndex)
		{
			HashValue(seed, _textures[textureIndex].get());
			HashSamplerDesc(seed, _samplerDescs[textureIndex]);
		}

		for (uint32 stageIndex = 0; stageIndex < ShaderStageCount; ++stageIndex)
		{
			for (uint32 slot = 0; slot < MaxConstantBufferSlots; ++slot)
			{
				const std::vector<uint8>& bytes = _constantData[stageIndex][slot].bytes;
				HashValue(seed, bytes.size());
				for (uint8 byte : bytes)
					HashValue(seed, byte);
			}
		}

		return seed;
	}

	bool Material::HasSameRenderState(const Material& rhs) const
	{
		if (this == &rhs)
			return true;

		if ((_surfaceData == rhs._surfaceData) == false || (_colorData == rhs._colorData) == false || _textureUVOffset.x != rhs._textureUVOffset.x || _textureUVOffset.y != rhs._textureUVOffset.y)
			return false;

		if (_vertexShader != rhs._vertexShader || _pixelShader != rhs._pixelShader || _topology != rhs._topology)
			return false;

		if ((_rasterizerDesc == rhs._rasterizerDesc) == false ||
			(_depthStencilDesc == rhs._depthStencilDesc) == false ||
			(_blendDesc == rhs._blendDesc) == false)
			return false;

		for (uint32 textureIndex = 0; textureIndex < TextureSlotCount; ++textureIndex)
		{
			if (_textures[textureIndex] != rhs._textures[textureIndex] ||
				(_samplerDescs[textureIndex] == rhs._samplerDescs[textureIndex]) == false)
				return false;
		}

		for (uint32 stageIndex = 0; stageIndex < ShaderStageCount; ++stageIndex)
		{
			for (uint32 slot = 0; slot < MaxConstantBufferSlots; ++slot)
			{
				if (_constantData[stageIndex][slot].bytes != rhs._constantData[stageIndex][slot].bytes)
					return false;
			}
		}

		return true;
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

	void Material::ApplySurfaceModePipelineState(SurfaceMode surfaceMode, DepthStencilDesc& depthStencilDesc, BlendDesc& blendDesc)
	{
		depthStencilDesc.depthEnable = true;

		switch (surfaceMode)
		{
		case SurfaceMode::Opaque:
		case SurfaceMode::Masked:
			depthStencilDesc.depthWriteEnable = true;
			blendDesc.blendEnable = false;
			blendDesc.srcBlend = BlendFactor::One;
			blendDesc.destBlend = BlendFactor::Zero;
			blendDesc.blendOp = BlendOp::Add;
			break;

		case SurfaceMode::Transparent:
			depthStencilDesc.depthWriteEnable = false;
			blendDesc.blendEnable = true;
			blendDesc.srcBlend = BlendFactor::SrcAlpha;
			blendDesc.destBlend = BlendFactor::InvSrcAlpha;
			blendDesc.blendOp = BlendOp::Add;
			break;

		default:
			GM_ASSERT_RETURN(false, "지원하지 않는 Material Surface Mode입니다.");
		}
	}

	/// Material Builder /////////////////////////////////////////////////////////////////////////////////////////////
	Material::MaterialBuilder::MaterialBuilder(Resources& resources) : _resources(resources)
	{
		Material::ApplySurfaceModePipelineState(_surfaceData.surfaceMode, _depthStencilDesc, _blendDesc);
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetSurfaceData(const MaterialSurfaceData& data)
	{
		SetShadingModel(data.shadingModel);
		SetSurfaceMode(data.surfaceMode);
		SetOutlineMode(data.outlineMode);
		SetEmissiveColor(data.emissiveColor);
		SetEmissiveIntensity(data.emissiveIntensity);
		SetAlphaCutoff(data.alphaCutoff);
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetColorData(const MaterialColorData& data)
	{
		GM_ASSERT_RETURN_VAL(data.mode < MaterialColorMode::Count, *this, "지원하지 않는 Material Color Mode입니다.");
		GM_ASSERT_RETURN_VAL(data.blendRatio >= 0.f && data.blendRatio <= 1.f, *this, "Material Color Blend Ratio는 0과 1 사이여야 합니다.");
		_colorData = data;
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetColorBlend(const Color& color, float ratio)
	{
		GM_ASSERT_RETURN_VAL(ratio >= 0.f && ratio <= 1.f, *this, "Material Color Blend Ratio는 0과 1 사이여야 합니다.");
		_colorData.mode = MaterialColorMode::Blend;
		_colorData.blendColor = color;
		_colorData.blendRatio = ratio;
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetOpacityGradient(const Color& lowColor, const Color& highColor)
	{
		_colorData.mode = MaterialColorMode::OpacityGradient;
		_colorData.opacityLowColor = lowColor;
		_colorData.opacityHighColor = highColor;
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetColorMultiplier(const Color& multiplier)
	{
		_colorData.colorMultiplier = multiplier;
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetShadingModel(ShadingModel shadingModel)
	{
		GM_ASSERT_RETURN_VAL(shadingModel < ShadingModel::Count, *this, "지원하지 않는 Material Shading Model입니다.");
		_surfaceData.shadingModel = shadingModel;
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetSurfaceMode(SurfaceMode surfaceMode)
	{
		GM_ASSERT_RETURN_VAL(surfaceMode < SurfaceMode::Count, *this, "지원하지 않는 Material Surface Mode입니다.");
		_surfaceData.surfaceMode = surfaceMode;
		Material::ApplySurfaceModePipelineState(surfaceMode, _depthStencilDesc, _blendDesc);
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetOutlineMode(OutlineMode outlineMode)
	{
		GM_ASSERT_RETURN_VAL(outlineMode < OutlineMode::Count, *this, "지원하지 않는 Material Outline Mode입니다.");
		_surfaceData.outlineMode = outlineMode;
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetEmissiveColor(const Color& color)
	{
		_surfaceData.emissiveColor = color;
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetEmissiveIntensity(float intensity)
	{
		GM_ASSERT_RETURN_VAL(intensity >= 0.f, *this, "Material Emissive Intensity는 0 이상이어야 합니다.");
		_surfaceData.emissiveIntensity = intensity;
		return *this;
	}

	Material::MaterialBuilder& Material::MaterialBuilder::SetAlphaCutoff(float alphaCutoff)
	{
		GM_ASSERT_RETURN_VAL(alphaCutoff >= 0.f && alphaCutoff <= 1.f, *this, "Material Alpha Cutoff은 0과 1 사이여야 합니다.");
		_surfaceData.alphaCutoff = alphaCutoff;
		return *this;
	}

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
