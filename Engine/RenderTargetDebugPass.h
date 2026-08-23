#pragma once

#include "GraphicsTypes.h"
#include "RenderTypes.h"
#include <memory>

namespace gm
{
	struct CameraViewInfo;
	class ConstantBuffer;
	class FullscreenRenderPass;
	class IGraphicsCommandContext;
	class IGraphicsResourceFactory;
	class Resources;
	class Shader;
	class Texture;

	class RenderTargetDebugPass
	{
	public:
		RenderTargetDebugPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory, FullscreenRenderPass& fullscreenRenderPass);
		~RenderTargetDebugPass();

		bool Initialize();
		void Render(const Texture& texture, RenderTargetDebugView view, const CameraViewInfo& viewInfo, float valueScale = 1.f);

	private:
		Resources&					_resources;
		IGraphicsCommandContext&		_commandContext;
		IGraphicsResourceFactory&	_resourceFactory;
		FullscreenRenderPass&		_fullscreenRenderPass;
		std::shared_ptr<Shader>		_pixelShader;
		std::unique_ptr<ConstantBuffer>	_constantBuffer;
		SamplerDesc					_pointSamplerDesc;
		SamplerDesc					_linearSamplerDesc;
	};
}
