#pragma once

#include "GraphicsTypes.h"
#include <memory>

namespace gm
{
	struct CameraViewInfo;
	struct DepthFogSettings;
	class ConstantBuffer;
	class FullscreenRenderPass;
	class IGraphicsCommandContext;
	class IGraphicsResourceFactory;
	class Resources;
	class Shader;
	class Texture;

	class DepthFogPass
	{
	public:
		DepthFogPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory, FullscreenRenderPass& fullscreenRenderPass);
		~DepthFogPass();

		bool Initialize();
		void Render(const Texture& sceneColor, const Texture& sceneDepth, Texture& output, const CameraViewInfo& viewInfo, const DepthFogSettings& settings);
		void BindForwardConstants(const DepthFogSettings& settings);

	private:
		void BindFogConstants(const DepthFogSettings& settings);

	private:
		Resources& _resources;
		IGraphicsCommandContext& _commandContext;
		IGraphicsResourceFactory& _resourceFactory;
		FullscreenRenderPass& _fullscreenRenderPass;
		std::shared_ptr<Shader> _pixelShader;
		std::unique_ptr<ConstantBuffer> _cameraBuffer;
		std::unique_ptr<ConstantBuffer> _fogBuffer;
		SamplerDesc _sceneColorSampler;
		SamplerDesc _depthSampler;
	};
}
