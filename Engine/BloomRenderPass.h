#pragma once

#include "GraphicsTypes.h"
#include <array>
#include <memory>

namespace gm
{
	struct BloomSettings
	{
		float threshold = 3.f;
		float softKnee = 0.5f;
		float intensity = 0.8f;
	};

	class ConstantBuffer;
	class FullscreenRenderPass;
	class IGraphicsCommandContext;
	class IGraphicsResourceFactory;
	class Resources;
	class Shader;
	class Texture;

	class BloomRenderPass
	{
	public:
		BloomRenderPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory, FullscreenRenderPass& fullscreenRenderPass);
		~BloomRenderPass();

		bool Initialize(uint32 width, uint32 height);
		void Render(const Texture& sceneColor, Texture& output);
		const Texture& GetBloomTexture() const { return *_bloomTextures[0]; }
		void SetSettings(const BloomSettings& settings) { _settings = settings; }
		const BloomSettings& GetSettings() const { return _settings; }

	private:
		void Downsample(const Texture& source, Texture& output, bool applyThreshold);
		void Upsample(const Texture& source, Texture& output);
		void Composite(const Texture& sceneColor, const Texture& bloomTexture, Texture& output);
		void UpdateConstants(const Texture& source, bool applyThreshold);

	private:
		static constexpr size_t LevelCount = 5;

		Resources& _resources;
		IGraphicsCommandContext& _commandContext;
		IGraphicsResourceFactory& _resourceFactory;
		FullscreenRenderPass& _fullscreenRenderPass;
		std::shared_ptr<Shader> _downsampleShader;
		std::shared_ptr<Shader> _upsampleShader;
		std::shared_ptr<Shader> _compositeShader;
		std::unique_ptr<ConstantBuffer> _constantBuffer;
		std::array<std::unique_ptr<Texture>, LevelCount> _bloomTextures;
		SamplerDesc _samplerDesc;
		BloomSettings _settings{};
	};
}
