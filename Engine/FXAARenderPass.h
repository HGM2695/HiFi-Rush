#pragma once

#include <memory>

namespace gm
{
	class ConstantBuffer;
	class FullscreenRenderPass;
	class IGraphicsCommandContext;
	class IGraphicsResourceFactory;
	class Resources;
	class Shader;
	class Texture;

	class FXAARenderPass
	{
	public:
		FXAARenderPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory, FullscreenRenderPass& fullscreenRenderPass);
		~FXAARenderPass();

		bool Initialize();
		void Render(const Texture& sceneColor);

	private:
		Resources& _resources;
		IGraphicsCommandContext& _commandContext;
		IGraphicsResourceFactory& _resourceFactory;
		FullscreenRenderPass& _fullscreenRenderPass;
		std::shared_ptr<Shader> _pixelShader;
		std::unique_ptr<ConstantBuffer> _constantBuffer;
	};
}
