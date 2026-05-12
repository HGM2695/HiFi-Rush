#include "D3D11GraphicsResourceFactory.h"
#include "D3D11GraphicsDevice.h"
#include "D3D11Mesh.h"
#include "D3D11Shader.h"
#include "D3D11TypeConverter.h"

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
}
