#pragma once

#include "EngineCore.h"

namespace gm
{
	class Shader;
	class Mesh;
	struct ShaderDesc;
	struct VertexLayoutDesc;
	struct MeshDesc;

	class IGraphicsResourceFactory
	{
	public:
		virtual ~IGraphicsResourceFactory() = default;

		virtual std::shared_ptr<Shader> CreateVertexShader(const ShaderDesc& shaderDesc, const VertexLayoutDesc& layoutDesc) = 0;
		virtual std::shared_ptr<Shader> CreatePixelShader(const ShaderDesc& shaderDesc) = 0;
		virtual std::shared_ptr<Mesh>	CreateMesh(const MeshDesc& meshDesc) = 0;
	};
}
