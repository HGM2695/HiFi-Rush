#include "SSAORenderPass.h"
#include "BuiltinGraphicsResources.h"
#include "CameraViewInfo.h"
#include "ConstantBuffer.h"
#include "FullscreenRenderPass.h"
#include "IGraphicsCommandContext.h"
#include "IGraphicsResourceFactory.h"
#include "Resources.h"
#include "Shader.h"
#include "Texture.h"
#include <array>
#include <cmath>
#include <random>

namespace gm
{
	namespace
	{
		inline constexpr uint32 SSAOSampleCount = 16;
		inline constexpr uint32 NoiseTextureSize = 4;

		struct SSAOConstantPS
		{
			Matrix inverseViewProjection = Matrix::Identity;
			Matrix projection = Matrix::Identity;
			Matrix viewMatrix = Matrix::Identity;
			std::array<Vector4, SSAOSampleCount> samples{};
			Vector2 noiseScale{};
			float radius = 0.15f;
			float fadeStart = 0.05f;
			float fadeEnd = 0.7f;
			float surfaceEpsilon = 0.01f;
			float strength = 1.25f;
			float padding = 0.f;
		};

		struct SSAOBlurConstantPS
		{
			Vector2 texelSize{};
			Vector2 padding{};
		};

		static_assert(sizeof(SSAOConstantPS) % 16 == 0);
		static_assert(sizeof(SSAOBlurConstantPS) % 16 == 0);

		std::array<Vector4, SSAOSampleCount> CreateSampleKernel()
		{
			std::mt19937 randomEngine(0x48524653u);
			std::uniform_real_distribution<float> randomScale(0.25f, 1.f);
			std::array<Vector3, SSAOSampleCount> directions
			{
				Vector3{ 1.f, 1.f, 1.f }, Vector3{ -1.f, -1.f, -1.f }, Vector3{ -1.f, 1.f, 1.f }, Vector3{ 1.f, -1.f, -1.f },
				Vector3{ 1.f, 1.f, -1.f }, Vector3{ -1.f, -1.f, 1.f }, Vector3{ -1.f, 1.f, -1.f }, Vector3{ 1.f, -1.f, 1.f },
				Vector3{ -1.f, 0.f, 0.f }, Vector3{ 1.f, 0.f, 0.f }, Vector3{ 0.f, -1.f, 0.f }, Vector3{ 0.f, 1.f, 0.f },
				Vector3{ 0.f, 0.f, -1.f }, Vector3{ 0.f, 0.f, 1.f }, Vector3{ 1.f, 0.f, 1.f }, Vector3{ -1.f, 0.f, 1.f }
			};
			std::array<Vector4, SSAOSampleCount> samples{};
			for (uint32 sampleIndex = 0; sampleIndex < SSAOSampleCount; ++sampleIndex)
			{
				Vector3 direction = directions[sampleIndex];
				direction.Normalize();
				const float scale = randomScale(randomEngine);
				samples[sampleIndex] = Vector4{ direction.x * scale, direction.y * scale, direction.z * scale, 0.f };
			}
			return samples;
		}

		const std::array<Vector4, SSAOSampleCount> SSAOSampleKernel = CreateSampleKernel();
	}

	SSAORenderPass::SSAORenderPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory, FullscreenRenderPass& fullscreenRenderPass)
		: _resources(resources), _commandContext(commandContext), _resourceFactory(resourceFactory), _fullscreenRenderPass(fullscreenRenderPass)
	{
		_pointClampSampler.filter = TextureFilter::Point;
		_linearWrapSampler.addressU = TextureAddressMode::Wrap;
		_linearWrapSampler.addressV = TextureAddressMode::Wrap;
	}

	SSAORenderPass::~SSAORenderPass() = default;

	bool SSAORenderPass::Initialize(uint32 width, uint32 height)
	{
		_width = width;
		_height = height;
		_halfWidth = (width + 1) / 2;
		_halfHeight = (height + 1) / 2;
		_ssaoPixelShader = _resources.Find<Shader>(BuiltinResourceKey::SSAOPS);
		_downsampleBlurPixelShader = _resources.Find<Shader>(BuiltinResourceKey::SSAODownsampleBlurPS);
		_upsampleBlurPixelShader = _resources.Find<Shader>(BuiltinResourceKey::SSAOUpsampleBlurPS);
		GM_ASSERT_RETURN_VAL(_ssaoPixelShader && _downsampleBlurPixelShader && _upsampleBlurPixelShader, false, "SSAO Shader를 찾지 못했습니다.");

		ConstantBufferDesc constantBufferDesc{};
		constantBufferDesc.size = sizeof(SSAOConstantPS);
		_ssaoConstantBuffer = _resourceFactory.CreateConstantBuffer(constantBufferDesc);
		GM_ASSERT_RETURN_VAL(_ssaoConstantBuffer, false, "SSAO Constant Buffer 생성에 실패했습니다.");
		constantBufferDesc.size = sizeof(SSAOBlurConstantPS);
		_blurConstantBuffer = _resourceFactory.CreateConstantBuffer(constantBufferDesc);
		GM_ASSERT_RETURN_VAL(_blurConstantBuffer, false, "SSAO Blur Constant Buffer 생성에 실패했습니다.");

		TextureDesc ambientOcclusionDesc{};
		ambientOcclusionDesc.width = width;
		ambientOcclusionDesc.height = height;
		ambientOcclusionDesc.format = TextureFormat::R32Float;
		ambientOcclusionDesc.bindUsage = TextureBindUsage::RenderTarget | TextureBindUsage::ShaderResource;
		_ambientOcclusionTexture = _resourceFactory.CreateTexture(ambientOcclusionDesc);
		GM_ASSERT_RETURN_VAL(_ambientOcclusionTexture, false, "Full Resolution SSAO Render Target 생성에 실패했습니다.");
		ambientOcclusionDesc.width = _halfWidth;
		ambientOcclusionDesc.height = _halfHeight;
		_blurTexture = _resourceFactory.CreateTexture(ambientOcclusionDesc);
		GM_ASSERT_RETURN_VAL(_blurTexture, false, "Half Resolution SSAO Blur Render Target 생성에 실패했습니다.");
		return CreateNoiseTexture();
	}

	bool SSAORenderPass::CreateNoiseTexture()
	{
		std::mt19937 randomEngine(0x5353414fu);
		std::uniform_real_distribution<float> randomSigned(-1.f, 1.f);
		std::array<uint32, NoiseTextureSize * NoiseTextureSize> noisePixels{};
		for (uint32& pixel : noisePixels)
		{
			Vector3 direction{ randomSigned(randomEngine), randomSigned(randomEngine), randomSigned(randomEngine) };
			direction.Normalize();
			const uint32 red = static_cast<uint32>(std::lround((direction.x * 0.5f + 0.5f) * 255.f));
			const uint32 green = static_cast<uint32>(std::lround((direction.y * 0.5f + 0.5f) * 255.f));
			const uint32 blue = static_cast<uint32>(std::lround((direction.z * 0.5f + 0.5f) * 255.f));
			pixel = red | (green << 8) | (blue << 16) | (255u << 24);
		}

		TextureDesc noiseDesc{};
		noiseDesc.width = NoiseTextureSize;
		noiseDesc.height = NoiseTextureSize;
		noiseDesc.format = TextureFormat::RGBA8UNorm;
		noiseDesc.bindUsage = TextureBindUsage::ShaderResource;
		noiseDesc.initialData = noisePixels.data();
		noiseDesc.initialDataRowPitch = NoiseTextureSize * sizeof(uint32);
		_noiseTexture = _resourceFactory.CreateTexture(noiseDesc);
		GM_ASSERT_RETURN_VAL(_noiseTexture, false, "SSAO Noise Texture 생성에 실패했습니다.");
		return true;
	}

	void SSAORenderPass::Render(const Texture& sceneDepth, const Texture& worldNormal, const CameraViewInfo& viewInfo)
	{
		_commandContext.ClearRenderTarget(*_ambientOcclusionTexture, Colors::White);
		_commandContext.ClearRenderTarget(*_blurTexture, Colors::White);
		_commandContext.SetViewport(Viewport{ 0.f, 0.f, static_cast<float>(_width), static_cast<float>(_height) });

		SSAOConstantPS constant{};
		constant.inverseViewProjection = (viewInfo.view * viewInfo.projection).Invert();
		constant.projection = viewInfo.projection;
		constant.viewMatrix = viewInfo.view;
		constant.samples = SSAOSampleKernel;
		constant.noiseScale = Vector2{ static_cast<float>(_width) / NoiseTextureSize, static_cast<float>(_height) / NoiseTextureSize };
		constant.radius = _settings.radius;
		constant.fadeStart = _settings.fadeStart;
		constant.fadeEnd = _settings.fadeEnd;
		constant.surfaceEpsilon = _settings.surfaceEpsilon;
		constant.strength = _settings.strength;
		_commandContext.UpdateConstantBuffer(*_ssaoConstantBuffer, &constant, sizeof(constant));
		_commandContext.BindConstantBuffer(ShaderStage::Pixel, 0, _ssaoConstantBuffer.get());
		_commandContext.BindRenderTarget(_ambientOcclusionTexture.get(), nullptr);
		_commandContext.BindShaderTexture(0, &sceneDepth);
		_commandContext.BindShaderTexture(1, &worldNormal);
		_commandContext.BindShaderTexture(2, _noiseTexture.get());
		_commandContext.BindSampler(0, &_pointClampSampler);
		_commandContext.BindSampler(1, &_linearWrapSampler);
		_fullscreenRenderPass.Draw(*_ssaoPixelShader);
		_commandContext.UnbindShaderTextures(0, 3);

		RenderDownsampleBlur();
		RenderUpsampleBlur();
	}

	void SSAORenderPass::RenderDownsampleBlur()
	{
		SSAOBlurConstantPS constant{};
		constant.texelSize = Vector2{ 1.f / static_cast<float>(_width), 1.f / static_cast<float>(_height) };
		_commandContext.UpdateConstantBuffer(*_blurConstantBuffer, &constant, sizeof(constant));
		_commandContext.BindConstantBuffer(ShaderStage::Pixel, 0, _blurConstantBuffer.get());
		_commandContext.SetViewport(Viewport{ 0.f, 0.f, static_cast<float>(_halfWidth), static_cast<float>(_halfHeight) });
		_commandContext.BindRenderTarget(_blurTexture.get(), nullptr);
		_commandContext.BindShaderTexture(0, _ambientOcclusionTexture.get());
		_commandContext.BindSampler(0, &_linearClampSampler);
		_fullscreenRenderPass.Draw(*_downsampleBlurPixelShader);
		_commandContext.UnbindShaderTextures(0, 1);
	}

	void SSAORenderPass::RenderUpsampleBlur()
	{
		SSAOBlurConstantPS constant{};
		constant.texelSize = Vector2{ 1.f / static_cast<float>(_halfWidth), 1.f / static_cast<float>(_halfHeight) };
		_commandContext.UpdateConstantBuffer(*_blurConstantBuffer, &constant, sizeof(constant));
		_commandContext.BindConstantBuffer(ShaderStage::Pixel, 0, _blurConstantBuffer.get());
		_commandContext.SetViewport(Viewport{ 0.f, 0.f, static_cast<float>(_width), static_cast<float>(_height) });
		_commandContext.BindRenderTarget(_ambientOcclusionTexture.get(), nullptr);
		_commandContext.BindShaderTexture(0, _blurTexture.get());
		_commandContext.BindSampler(0, &_linearClampSampler);
		_fullscreenRenderPass.Draw(*_upsampleBlurPixelShader);
		_commandContext.UnbindShaderTextures(0, 1);
	}
}
