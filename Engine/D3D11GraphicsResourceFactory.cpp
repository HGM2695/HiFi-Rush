#include "D3D11GraphicsResourceFactory.h"
#include "D3D11GraphicsDevice.h"
#include "D3D11Mesh.h"
#include "D3D11Shader.h"
#include "D3D11TypeConverter.h"
#include "D3D11PipelineState.h"
#include "D3D11Texture.h"
#include "D3D11Sampler.h"
#include "D3D11ConstantBuffer.h"

namespace gm
{
	D3D11GraphicsResourceFactory::D3D11GraphicsResourceFactory(D3D11GraphicsDevice& graphicsDevice)
		: _graphicsDevice(graphicsDevice)
	{
	}

	std::shared_ptr<Shader> D3D11GraphicsResourceFactory::CreateVertexShader(const ShaderDesc& shaderDesc, const VertexLayoutDesc& layoutDesc)
	{
		D3D11VertexShaderDesc desc{};
		desc.filePath = shaderDesc.filePath;
		desc.entryPoint = shaderDesc.entryPoint;
		desc.target = shaderDesc.target;
		desc.device = _graphicsDevice.GetNativeDevice();
		desc.inputElements = ToD3D11InputElements(layoutDesc);

		return D3D11VertexShader::Create(desc);
	}

	std::shared_ptr<Shader> D3D11GraphicsResourceFactory::CreatePixelShader(const ShaderDesc& shaderDesc)
	{
		D3D11PixelShaderDesc desc{};
		desc.filePath = shaderDesc.filePath;
		desc.entryPoint = shaderDesc.entryPoint;
		desc.target = shaderDesc.target;
		desc.device = _graphicsDevice.GetNativeDevice();

		return D3D11PixelShader::Create(desc);
	}

	std::shared_ptr<Mesh> D3D11GraphicsResourceFactory::CreateMesh(const MeshDesc& meshDesc)
	{
		D3D11MeshDesc desc{};
		desc.device = _graphicsDevice.GetNativeDevice();
		desc.topology = meshDesc.topology;
		desc.vertexData = meshDesc.vertexData;
		desc.vertexCount = meshDesc.vertexCount;
		desc.vertexStride = meshDesc.vertexStride;
		desc.indexData = meshDesc.indexData;
		desc.indexCount = meshDesc.indexCount;

		return D3D11Mesh::Create(desc);
	}

	std::shared_ptr<PipelineState> D3D11GraphicsResourceFactory::CraetePipelineState(const PipelineStateDesc& pipelineStateDesc)
	{
		D3D11PipelineStateDesc desc{};
		desc.vertexShader = pipelineStateDesc.vertexShader;
		desc.pixelShader = pipelineStateDesc.pixelShader;
		desc.topology = pipelineStateDesc.topology;
		desc.rasterizerDesc = pipelineStateDesc.rasterizerDesc;
		desc.depthStencilDesc = pipelineStateDesc.depthStencilDesc;
		desc.blendDesc = pipelineStateDesc.blendDesc;
		desc.device = _graphicsDevice.GetNativeDevice();

		return D3D11PipelineState::Create(desc);
	}

	std::shared_ptr<Texture> D3D11GraphicsResourceFactory::CreateTexture(const TextureDesc& textureDesc)
	{
		D3D11TextureDesc desc{};
		desc.path = textureDesc.path;
		desc.device = _graphicsDevice.GetNativeDevice();

		return D3D11Texture::Create(desc);
	}

	std::shared_ptr<Sampler> D3D11GraphicsResourceFactory::CreateSampler(const SamplerDesc& samplerDesc)
	{
		D3D11SamplerDesc desc{};
		desc.filter = samplerDesc.filter;
		desc.addressU = samplerDesc.addressU;
		desc.addressV = samplerDesc.addressV;
		desc.addressW = samplerDesc.addressW;
		desc.device = _graphicsDevice.GetNativeDevice();

		return D3D11Sampler::Create(desc);
	}

	std::unique_ptr<ConstantBuffer> D3D11GraphicsResourceFactory::CreateConstantBuffer(const ConstantBufferDesc& constantBufferDesc)
	{
		D3D11ConstantBufferDesc desc{};
		desc.size = constantBufferDesc.size;
		desc.initialData = constantBufferDesc.initialData;
		desc.device = _graphicsDevice.GetNativeDevice();

		return D3D11ConstantBuffer::Create(desc);
	}
}
