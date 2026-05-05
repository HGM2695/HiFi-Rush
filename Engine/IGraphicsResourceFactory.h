#pragma once

#include "EngineCore.h"
#include "Mesh.h"
#include "Shader.h"
#include "VertexLayout.h"

namespace gm
{
	struct MeshDesc
	{
		PrimitiveTopology	topology = PrimitiveTopology::TriangleList;

		const void*			vertexData = nullptr;
		uint32				vertexCount = 0;
		uint32				vertexStride = 0;

		const uint32*		indexData = nullptr;
		uint32				indexCount = 0;
	};

	class IGraphicsResourceFactory
	{
	public:
		virtual ~IGraphicsResourceFactory() = default;

		virtual std::shared_ptr<Shader> CreateVertexShader(const ShaderDesc& shaderDesc, const VertexLayoutDesc& layoutDesc) = 0;
		virtual std::shared_ptr<Shader> CreatePixelShader(const ShaderDesc& shaderDesc) = 0;
		virtual std::shared_ptr<Mesh>	CreateMesh(const MeshDesc& meshDesc) = 0;
	};
}
