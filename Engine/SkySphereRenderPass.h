#pragma once

#include "CameraViewInfo.h"
#include "ConstantBufferPool.h"
#include <memory>

namespace gm
{
	class IGraphicsCommandContext;
	class IGraphicsResourceFactory;
	class Resources;
	class Shader;
	class StaticMesh;
	class Texture;

	class SkySphereRenderPass
	{
	public:
		SkySphereRenderPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory);
		~SkySphereRenderPass();

		bool Initialize();
		void SetSkySphere(const std::shared_ptr<StaticMesh>& staticMesh, const std::shared_ptr<Texture>& texture);
		void ClearSkySphere();
		void Render(const CameraViewInfo& viewInfo);

	private:
		IGraphicsCommandContext&	_commandContext;
		ConstantBufferPool		_constantBufferPool;
		std::shared_ptr<Shader>	_vertexShader;
		std::shared_ptr<Shader>	_pixelShader;
		std::shared_ptr<StaticMesh> _staticMesh;
		std::shared_ptr<Texture>	_texture;
	};
}
