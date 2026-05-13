#pragma once

#include "IGraphicsResourceFactory.h"

namespace gm
{
	class D3D11GraphicsDevice;

	class D3D11GraphicsResourceFactory : public IGraphicsResourceFactory
	{
	public:
		explicit D3D11GraphicsResourceFactory(D3D11GraphicsDevice& graphicsDevice);
		virtual ~D3D11GraphicsResourceFactory() = default;

		virtual std::shared_ptr<Shader>			CreateVertexShader(const ShaderDesc& shaderDesc, const VertexLayoutDesc& layoutDesc) override;
		virtual std::shared_ptr<Shader>			CreatePixelShader(const ShaderDesc& shaderDesc) override;
		virtual std::shared_ptr<Mesh>			CreateMesh(const MeshDesc& meshDesc) override;
		virtual std::shared_ptr<PipelineState>	CraetePipelineState(const PipelineStateDesc& pipelineStateDesc) override;
		virtual std::shared_ptr<Texture>		CreateTexture(const TextureDesc& textureDesc) override;

	private:
		D3D11GraphicsDevice& _graphicsDevice;
	};
}
