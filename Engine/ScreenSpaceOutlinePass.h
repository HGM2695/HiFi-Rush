#pragma once

#include "GraphicsTypes.h"
#include "MathTypes.h"
#include <memory>

namespace gm
{
	struct ScreenSpaceOutlineSettings
	{
		float depthThreshold = 0.005f;
		float normalThreshold = 0.2f;
		Color color = Colors::Black;
		float thickness = 0.5f;
	};

	struct CameraViewInfo;
	class ConstantBuffer;
	class FullscreenRenderPass;
	class IGraphicsCommandContext;
	class IGraphicsResourceFactory;
	class Resources;
	class Shader;
	class Texture;

	class ScreenSpaceOutlinePass
	{
	public:
		ScreenSpaceOutlinePass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory, FullscreenRenderPass& fullscreenRenderPass);
		~ScreenSpaceOutlinePass();

		bool Initialize(uint32 width, uint32 height);
		void Render(const Texture& sceneColor, const Texture& sceneDepth, const Texture& worldNormal, const Texture& materialData, Texture& output, const CameraViewInfo& viewInfo, uint32 width, uint32 height);
		void RenderDebug(const Texture& sceneColor, const Texture& sceneDepth, const Texture& worldNormal, const Texture& materialData, const CameraViewInfo& viewInfo, uint32 width, uint32 height);
		const Texture& GetDebugTexture() const { return *_debugTexture; }
		void SetSettings(const ScreenSpaceOutlineSettings& settings) { _settings = settings; }
		const ScreenSpaceOutlineSettings& GetSettings() const { return _settings; }

	private:
		void Draw(const Texture& sceneColor, const Texture& sceneDepth, const Texture& worldNormal, const Texture& materialData, Texture& output, const CameraViewInfo& viewInfo, uint32 width, uint32 height, bool debugOutput);

	private:
		Resources& _resources;
		IGraphicsCommandContext& _commandContext;
		IGraphicsResourceFactory& _resourceFactory;
		FullscreenRenderPass& _fullscreenRenderPass;
		std::shared_ptr<Shader> _pixelShader;
		std::unique_ptr<ConstantBuffer> _constantBuffer;
		std::unique_ptr<Texture> _debugTexture;
		SamplerDesc _pointClampSampler;
		ScreenSpaceOutlineSettings _settings{};
	};
}
