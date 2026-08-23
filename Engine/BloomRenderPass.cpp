#include "BloomRenderPass.h"
#include "BuiltinGraphicsResources.h"
#include "ConstantBuffer.h"
#include "FullscreenRenderPass.h"
#include "IGraphicsCommandContext.h"
#include "IGraphicsResourceFactory.h"
#include "Resources.h"
#include "Shader.h"
#include "Texture.h"

#include <algorithm>

namespace gm
{
	namespace
	{
		struct BloomConstantPS
		{
			Vector2 sourceTexelSize{};
			float threshold = 3.f;
			float softKnee = 0.5f;
			float intensity = 1.f;
			uint32 applyThreshold = 0;
			Vector2 padding{};
		};

		static_assert(sizeof(BloomConstantPS) % 16 == 0);
	}

	BloomRenderPass::BloomRenderPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory, FullscreenRenderPass& fullscreenRenderPass)
		: _resources(resources), _commandContext(commandContext), _resourceFactory(resourceFactory), _fullscreenRenderPass(fullscreenRenderPass)
	{
	}

	BloomRenderPass::~BloomRenderPass() = default;

	bool BloomRenderPass::Initialize(uint32 width, uint32 height)
	{
		_downsampleShader = _resources.Find<Shader>(BuiltinResourceKey::BloomDownsamplePS);
		_upsampleShader = _resources.Find<Shader>(BuiltinResourceKey::BloomUpsamplePS);
		_compositeShader = _resources.Find<Shader>(BuiltinResourceKey::BloomCompositePS);
		GM_ASSERT_RETURN_VAL(_downsampleShader && _upsampleShader && _compositeShader, false, "Bloom Pixel Shader를 찾지 못했습니다.");

		ConstantBufferDesc constantBufferDesc{};
		constantBufferDesc.size = sizeof(BloomConstantPS);
		_constantBuffer = _resourceFactory.CreateConstantBuffer(constantBufferDesc);
		GM_ASSERT_RETURN_VAL(_constantBuffer, false, "Bloom Constant Buffer 생성에 실패했습니다.");

		TextureDesc textureDesc{};
		textureDesc.width = width;
		textureDesc.height = height;
		textureDesc.format = TextureFormat::RGBA16Float;
		textureDesc.bindUsage = TextureBindUsage::RenderTarget | TextureBindUsage::ShaderResource;
		for (std::unique_ptr<Texture>& texture : _bloomTextures)
		{
			textureDesc.width = std::max(textureDesc.width / 2, 1u);
			textureDesc.height = std::max(textureDesc.height / 2, 1u);
			texture = _resourceFactory.CreateTexture(textureDesc);
			GM_ASSERT_RETURN_VAL(texture, false, "Bloom Downsample Texture 생성에 실패했습니다.");
		}
		return true;
	}

	void BloomRenderPass::Render(const Texture& sceneColor, Texture& output)
	{
		const Texture* source = &sceneColor;
		for (size_t levelIndex = 0; levelIndex < LevelCount; ++levelIndex)
		{
			Texture& target = *_bloomTextures[levelIndex];
			_commandContext.ClearRenderTarget(target, Colors::Transparent);
			Downsample(*source, target, levelIndex == 0);
			source = &target;
		}

		for (size_t levelIndex = LevelCount - 1; levelIndex > 0; --levelIndex)
			Upsample(*_bloomTextures[levelIndex], *_bloomTextures[levelIndex - 1]);

		Composite(sceneColor, *_bloomTextures[0], output);
	}

	void BloomRenderPass::Downsample(const Texture& source, Texture& output, bool applyThreshold)
	{
		UpdateConstants(source, applyThreshold);
		_commandContext.SetViewport(Viewport{ 0.f, 0.f, static_cast<float>(output.GetWidth()), static_cast<float>(output.GetHeight()) });
		_commandContext.BindRenderTarget(&output, nullptr);
		_commandContext.BindShaderTexture(0, &source);
		_commandContext.BindSampler(0, &_samplerDesc);
		_fullscreenRenderPass.Draw(*_downsampleShader);
		_commandContext.UnbindShaderTextures(0, 1);
	}

	void BloomRenderPass::Upsample(const Texture& source, Texture& output)
	{
		UpdateConstants(source, false);
		_commandContext.SetViewport(Viewport{ 0.f, 0.f, static_cast<float>(output.GetWidth()), static_cast<float>(output.GetHeight()) });
		_commandContext.BindRenderTarget(&output, nullptr);
		_commandContext.BindShaderTexture(0, &source);
		_commandContext.BindSampler(0, &_samplerDesc);
		BlendDesc additiveBlend{};
		additiveBlend.blendEnable = true;
		additiveBlend.srcBlend = BlendFactor::One;
		additiveBlend.destBlend = BlendFactor::One;
		_fullscreenRenderPass.Draw(*_upsampleShader, additiveBlend);
		_commandContext.UnbindShaderTextures(0, 1);
	}

	void BloomRenderPass::Composite(const Texture& sceneColor, const Texture& bloomTexture, Texture& output)
	{
		UpdateConstants(bloomTexture, false);
		_commandContext.SetViewport(Viewport{ 0.f, 0.f, static_cast<float>(output.GetWidth()), static_cast<float>(output.GetHeight()) });
		_commandContext.BindRenderTarget(&output, nullptr);
		_commandContext.BindShaderTexture(0, &sceneColor);
		_commandContext.BindShaderTexture(1, &bloomTexture);
		_commandContext.BindSampler(0, &_samplerDesc);
		_fullscreenRenderPass.Draw(*_compositeShader);
		_commandContext.UnbindShaderTextures(0, 2);
	}

	void BloomRenderPass::UpdateConstants(const Texture& source, bool applyThreshold)
	{
		BloomConstantPS constant{};
		constant.sourceTexelSize = Vector2{ 1.f / static_cast<float>(source.GetWidth()), 1.f / static_cast<float>(source.GetHeight()) };
		constant.threshold = _settings.threshold;
		constant.softKnee = _settings.softKnee;
		constant.intensity = _settings.intensity;
		constant.applyThreshold = applyThreshold ? 1u : 0u;
		_commandContext.UpdateConstantBuffer(*_constantBuffer, &constant, sizeof(constant));
		_commandContext.BindConstantBuffer(ShaderStage::Pixel, 0, _constantBuffer.get());
	}
}
