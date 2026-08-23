#pragma once

#include "GraphicsTypes.h"
#include <memory>

namespace gm
{
	struct CameraViewInfo;
	struct CascadedShadowRenderData;
	struct LightRenderData;
	class ConstantBuffer;
	class FullscreenRenderPass;
	class GBuffer;
	class IGraphicsCommandContext;
	class IGraphicsResourceFactory;
	class Resources;
	class Shader;
	class Texture;

	class DeferredCompositionPass
	{
	public:
		DeferredCompositionPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory, FullscreenRenderPass& fullscreenRenderPass);
		~DeferredCompositionPass();

		bool Initialize();
		void Render(const GBuffer& gBuffer, const Texture& sceneDepth, const Texture& screenSpaceAmbientOcclusion, Texture& sceneColor, const CameraViewInfo& viewInfo, const LightRenderData& lightData, const CascadedShadowRenderData& shadowData, const Texture& cascadedShadowTexture, const Texture* spotLightCookieTexture, bool showCascadeDebugColors);

	private:
		Resources&						_resources;
		IGraphicsCommandContext&		_commandContext;
		IGraphicsResourceFactory&		_resourceFactory;
		FullscreenRenderPass&			_fullscreenRenderPass;
		std::shared_ptr<Shader>			_pixelShader;
		std::unique_ptr<ConstantBuffer>	_cameraBuffer;
		std::unique_ptr<ConstantBuffer>	_lightBuffer;
		std::unique_ptr<ConstantBuffer>	_shadowBuffer;
		SamplerDesc						_gBufferSampler;
		SamplerDesc						_spotCookieSampler;
		SamplerDesc						_shadowSampler;
		SamplerDesc						_ambientOcclusionSampler;
	};
}
