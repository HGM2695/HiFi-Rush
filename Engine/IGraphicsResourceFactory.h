#pragma once

#include "EngineCore.h"

namespace gm
{
	class Shader;
	class Mesh;
	class PipelineState;
	class Texture;
	class Sampler;
	struct ShaderDesc;
	struct VertexLayoutDesc;
	struct MeshDesc;
	struct PipelineStateDesc;
	struct TextureDesc;
	struct SamplerDesc;

	class IGraphicsResourceFactory
	{
	public:
		virtual ~IGraphicsResourceFactory() = default;

		virtual std::shared_ptr<Shader>			CreateVertexShader(const ShaderDesc& shaderDesc, const VertexLayoutDesc& layoutDesc) = 0;
		virtual std::shared_ptr<Shader>			CreatePixelShader(const ShaderDesc& shaderDesc) = 0;
		virtual std::shared_ptr<Mesh>			CreateMesh(const MeshDesc& meshDesc) = 0;
		virtual std::shared_ptr<PipelineState>	CraetePipelineState(const PipelineStateDesc& pipelinestateDesc) = 0;
		virtual std::shared_ptr<Texture>		CreateTexture(const TextureDesc& textureDesc) = 0;
		virtual std::shared_ptr<Sampler>		CreateSampler(const SamplerDesc& samplerDesc) = 0;
	};
}
