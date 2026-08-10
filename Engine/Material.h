#pragma once

#include "GraphicsTypes.h"
#include <array>
#include <cstddef>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace gm
{
	class Resources;
	class Shader;
	class Texture;

	// Material은 Builder 패턴으로 생성합니다.
	class Material final
	{
	public:
		class MaterialBuilder;

		struct ConstantSlot
		{
			std::vector<uint8> bytes;

			bool		IsValid() const { return bytes.empty() == false; }
			const void* Data() const { return bytes.data(); }
			uint32		Size() const { return static_cast<uint32>(bytes.size()); }
		};

		using ConstantSlots = std::array<ConstantSlot, MaxConstantBufferSlots>;
		using ConstantSlotsByShader = std::array<ConstantSlots, ShaderStageCount>;

		explicit Material(const MaterialBuilder& builder);

		std::shared_ptr<Shader>	GetVertexShader() const { return _vertexShader; }
		std::shared_ptr<Shader>	GetPixelShader() const { return _pixelShader; }
		PrimitiveTopology		GetTopology() const { return _topology; }
		const RasterizerDesc&	GetRasterizerDesc() const { return _rasterizerDesc; }
		const DepthStencilDesc&	GetDepthStencilDesc() const { return _depthStencilDesc; }
		const BlendDesc&		GetBlendDesc() const { return _blendDesc; }
		std::shared_ptr<Texture> GetTexture(TextureSlot slot) const;
		const SamplerDesc&		GetSamplerDesc(TextureSlot slot) const;
		const ConstantSlot&		GetConstantSlot(ShaderStage stage, uint32 slot) const;
		const ConstantSlots&	GetConstantSlots(ShaderStage stage) const;
		size_t					GetRenderStateHash() const;
		bool					HasSameRenderState(const Material& rhs) const;

		void					SetVertexShader(const std::shared_ptr<Shader>& shader);
		void					SetPixelShader(const std::shared_ptr<Shader>& shader);
		void					SetTopology(PrimitiveTopology topology);
		void					SetRasterizerDesc(const RasterizerDesc& desc);
		void					SetDepthStencilDesc(const DepthStencilDesc& desc);
		void					SetBlendDesc(const BlendDesc& desc);
		void					SetTexture(TextureSlot slot, const std::shared_ptr<Texture>& texture);
		void					SetSamplerDesc(TextureSlot slot, const SamplerDesc& desc);
		void					SetConstantData(ShaderStage stage, uint32 slot, const void* data, uint32 size);

		template <typename T>
		void SetConstantData(ShaderStage stage, uint32 slot, const T& data)
		{
			SetConstantData(stage, slot, &data, static_cast<uint32>(sizeof(T)));
		}

	private:
		std::array<std::shared_ptr<Texture>, TextureSlotCount>	_textures{};
		std::array<SamplerDesc, TextureSlotCount>				_samplerDescs{};
		ConstantSlotsByShader									_constantData{};
		std::shared_ptr<Shader>									_vertexShader = nullptr;
		std::shared_ptr<Shader>									_pixelShader = nullptr;
		PrimitiveTopology										_topology = PrimitiveTopology::TriangleList;
		RasterizerDesc											_rasterizerDesc{};
		DepthStencilDesc										_depthStencilDesc{};
		BlendDesc												_blendDesc{};

	public:
		class MaterialBuilder
		{
			friend class Material;

		public:
			explicit MaterialBuilder(Resources& resources);

			MaterialBuilder&	SetVertexShader(const std::wstring& key);
			MaterialBuilder&	SetPixelShader(const std::wstring& key);
			MaterialBuilder&	SetTexture(TextureSlot slot, const std::wstring& key);
			MaterialBuilder&	SetSampler(TextureSlot slot, const SamplerDesc& desc);
			MaterialBuilder&	SetTopology(PrimitiveTopology topology);
			MaterialBuilder&	SetRasterizer(const RasterizerDesc& desc);
			MaterialBuilder&	SetDepthStencil(const DepthStencilDesc& desc);
			MaterialBuilder&	SetBlend(const BlendDesc& desc);

			MaterialBuilder&	SetFillMode(FillMode mode);
			MaterialBuilder&	SetCullMode(CullMode mode);
			MaterialBuilder&	SetFrontCounterClockwise(bool enable);
			MaterialBuilder&	SetDepthClipEnable(bool enable);

			MaterialBuilder&	SetDepthEnable(bool enable);
			MaterialBuilder&	SetDepthWriteEnable(bool enable);
			MaterialBuilder&	SetDepthFunc(CompareFunc func);
			MaterialBuilder&	SetStencilEnable(bool enable);

			MaterialBuilder&	SetBlendEnable(bool enable);
			MaterialBuilder&	SetBlendFactor(BlendFactor srcBlend, BlendFactor destBlend);
			MaterialBuilder&	SetBlendOp(BlendOp op);

			MaterialBuilder&	SetSamplerFilter(TextureSlot slot, TextureFilter filter);
			MaterialBuilder&	SetSamplerAddressMode(TextureSlot slot, TextureAddressMode addressMode);
			MaterialBuilder&	SetSamplerAddressMode(TextureSlot slot, TextureAddressMode addressU, TextureAddressMode addressV, TextureAddressMode addressW);
			MaterialBuilder&	SetConstantData(ShaderStage stage, uint32 slot, const void* data, uint32 size);

			template <typename T>
			MaterialBuilder& SetConstantData(ShaderStage stage, uint32 slot, const T& data)
			{
				return SetConstantData(stage, slot, &data, static_cast<uint32>(sizeof(T)));
			}

			Material			Build() const;

		private:
			Resources&													_resources;
			std::array<std::shared_ptr<gm::Texture>, TextureSlotCount>	_textures{};
			std::array<SamplerDesc, TextureSlotCount>					_samplerDescs{};
			ConstantSlotsByShader										_constantData{};
			std::shared_ptr<Shader>										_vertexShader;
			std::shared_ptr<Shader>										_pixelShader;
			PrimitiveTopology											_topology = PrimitiveTopology::TriangleList;
			RasterizerDesc												_rasterizerDesc{};
			DepthStencilDesc											_depthStencilDesc{};
			BlendDesc													_blendDesc{};
		};
	};
}
