#pragma once

#include <memory>

namespace gm
{
	struct ToneMappingSettings;
	class ConstantBuffer;
	class FullscreenRenderPass;
	class IGraphicsCommandContext;
	class IGraphicsResourceFactory;
	class Resources;
	class Shader;
	class Texture;

	class ToneMappingPass
	{
	public:
		ToneMappingPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory, FullscreenRenderPass& fullscreenRenderPass);
		~ToneMappingPass();

		bool Initialize();
		void Render(const Texture& sceneColor, const ToneMappingSettings& settings);

	private:
		Resources& _resources;
		IGraphicsCommandContext& _commandContext;
		IGraphicsResourceFactory& _resourceFactory;
		FullscreenRenderPass& _fullscreenRenderPass;
		std::shared_ptr<Shader> _pixelShader;
		std::unique_ptr<ConstantBuffer> _constantBuffer;
	};
}
