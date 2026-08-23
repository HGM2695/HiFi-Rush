#pragma once

#include "GraphicsTypes.h"
#include <memory>

namespace gm
{
	struct SSAOSettings
	{
		float radius = 0.15f;
		float fadeStart = 0.05f;
		float fadeEnd = 0.7f;
		float surfaceEpsilon = 0.01f;
		float strength = 1.25f;
	};

	struct CameraViewInfo;
	class ConstantBuffer;
	class FullscreenRenderPass;
	class IGraphicsCommandContext;
	class IGraphicsResourceFactory;
	class Resources;
	class Shader;
	class Texture;

	class SSAORenderPass
	{
	public:
		SSAORenderPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory, FullscreenRenderPass& fullscreenRenderPass);
		~SSAORenderPass();

		bool Initialize(uint32 width, uint32 height);
		void Render(const Texture& sceneDepth, const Texture& worldNormal, const CameraViewInfo& viewInfo);
		const Texture& GetAmbientOcclusionTexture() const { return *_ambientOcclusionTexture; }
		void SetSettings(const SSAOSettings& settings) { _settings = settings; }
		const SSAOSettings& GetSettings() const { return _settings; }

	private:
		bool CreateNoiseTexture();
		void RenderDownsampleBlur();
		void RenderUpsampleBlur();

	private:
		Resources& _resources;
		IGraphicsCommandContext& _commandContext;
		IGraphicsResourceFactory& _resourceFactory;
		FullscreenRenderPass& _fullscreenRenderPass;
		std::shared_ptr<Shader> _ssaoPixelShader;
		std::shared_ptr<Shader> _downsampleBlurPixelShader;
		std::shared_ptr<Shader> _upsampleBlurPixelShader;
		std::unique_ptr<ConstantBuffer> _ssaoConstantBuffer;
		std::unique_ptr<ConstantBuffer> _blurConstantBuffer;
		std::unique_ptr<Texture> _noiseTexture;
		std::unique_ptr<Texture> _ambientOcclusionTexture;
		std::unique_ptr<Texture> _blurTexture;
		SamplerDesc _pointClampSampler;
		SamplerDesc _linearWrapSampler;
		SamplerDesc _linearClampSampler;
		SSAOSettings _settings{};
		uint32 _width = 0;
		uint32 _height = 0;
		uint32 _halfWidth = 0;
		uint32 _halfHeight = 0;
	};
}
