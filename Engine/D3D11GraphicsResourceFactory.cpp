#include "D3D11GraphicsResourceFactory.h"
#include "D3D11GraphicsDevice.h"
#include "D3D11Mesh.h"
#include "D3D11Shader.h"

namespace gm
{
	namespace
	{
		const char* ToSemanticName(VertexElementSemantic semantic);
		DXGI_FORMAT ToDXGIFormat(VertexElementFormat format);
		std::vector<D3D11_INPUT_ELEMENT_DESC> ToD3D11InputElements(const VertexLayoutDesc& layoutDesc);
	}

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

	// helper
	namespace
	{
		const char* ToSemanticName(VertexElementSemantic semantic)
		{
			switch (semantic)
			{
			case VertexElementSemantic::Position:
				return "POSITION";
			case VertexElementSemantic::Normal:
				return "NORMAL";
			case VertexElementSemantic::TexCoord:
				return "TEXCOORD";
			case VertexElementSemantic::Color:
				return "COLOR";
			default:
				return "";
			}
		}

		DXGI_FORMAT ToDXGIFormat(VertexElementFormat format)
		{
			switch (format)
			{
			case VertexElementFormat::Float2:
				return DXGI_FORMAT_R32G32_FLOAT;
			case VertexElementFormat::Float3:
				return DXGI_FORMAT_R32G32B32_FLOAT;
			case VertexElementFormat::Float4:
				return DXGI_FORMAT_R32G32B32A32_FLOAT;
			default:
				return DXGI_FORMAT_UNKNOWN;
			}
		}

		std::vector<D3D11_INPUT_ELEMENT_DESC> ToD3D11InputElements(const VertexLayoutDesc& layoutDesc)
		{
			std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements;
			inputElements.reserve(layoutDesc.elements.size());

			for (const VertexElementDesc& element : layoutDesc.elements)
			{
				inputElements.push_back({
					ToSemanticName(element.semantic),
					element.semanticIndex,
					ToDXGIFormat(element.format),
					0,
					element.offset,
					D3D11_INPUT_PER_VERTEX_DATA,
					0
					});
			}

			return inputElements;
		}
	}
}
