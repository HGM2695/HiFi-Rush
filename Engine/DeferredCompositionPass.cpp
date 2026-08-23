#include "DeferredCompositionPass.h"
#include "BuiltinGraphicsResources.h"
#include "CameraViewInfo.h"
#include "ConstantBuffer.h"
#include "FullscreenRenderPass.h"
#include "GBuffer.h"
#include "IGraphicsCommandContext.h"
#include "IGraphicsResourceFactory.h"
#include "LightRenderData.h"
#include "Resources.h"
#include "Shader.h"
#include "ShadowTypes.h"
#include "Texture.h"
namespace gm
{
	namespace
	{
		struct DeferredCompositionConstantPS
		{
			Matrix inverseViewProjection = Matrix::Identity;
			Matrix viewMatrix = Matrix::Identity;
			uint32 showCascadeDebugColors = 0;
			Vector3 padding{};
		};
	}

	DeferredCompositionPass::DeferredCompositionPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory, FullscreenRenderPass& fullscreenRenderPass)
		: _resources(resources), _commandContext(commandContext), _resourceFactory(resourceFactory), _fullscreenRenderPass(fullscreenRenderPass)
	{
		_gBufferSampler.filter = TextureFilter::Point;
		_shadowSampler.filter = TextureFilter::Point;
	}

	DeferredCompositionPass::~DeferredCompositionPass() = default;

	bool DeferredCompositionPass::Initialize()
	{
		_pixelShader = _resources.Find<Shader>(BuiltinResourceKey::DeferredCompositionPS);
		GM_ASSERT_RETURN_VAL(_pixelShader, false, "Deferred Composition Pixel Shader를 찾지 못했습니다.");

		ConstantBufferDesc desc{};
		desc.size = sizeof(DeferredCompositionConstantPS);
		_cameraBuffer = _resourceFactory.CreateConstantBuffer(desc);
		GM_ASSERT_RETURN_VAL(_cameraBuffer, false, "Deferred Composition Camera Constant Buffer 생성에 실패했습니다.");

		desc.size = sizeof(LightRenderData);
		_lightBuffer = _resourceFactory.CreateConstantBuffer(desc);
		GM_ASSERT_RETURN_VAL(_lightBuffer, false, "Deferred Composition Light Constant Buffer 생성에 실패했습니다.");

		desc.size = sizeof(CascadedShadowRenderData);
		_shadowBuffer = _resourceFactory.CreateConstantBuffer(desc);
		GM_ASSERT_RETURN_VAL(_shadowBuffer, false, "Deferred Composition Shadow Constant Buffer 생성에 실패했습니다.");
		return true;
	}

	void DeferredCompositionPass::Render(const GBuffer& gBuffer, const Texture& sceneDepth, const Texture& screenSpaceAmbientOcclusion, Texture& sceneColor, const CameraViewInfo& viewInfo, const LightRenderData& lightData, const CascadedShadowRenderData& shadowData, const Texture& cascadedShadowTexture, const Texture* spotLightCookieTexture, bool showCascadeDebugColors)
	{
		DeferredCompositionConstantPS constant{};
		constant.inverseViewProjection = (viewInfo.view * viewInfo.projection).Invert();
		constant.viewMatrix = viewInfo.view;
		constant.showCascadeDebugColors = showCascadeDebugColors ? 1u : 0u;
		_commandContext.UpdateConstantBuffer(*_cameraBuffer, &constant, sizeof(constant));
		_commandContext.BindConstantBuffer(ShaderStage::Pixel, 0, _cameraBuffer.get());
		_commandContext.UpdateConstantBuffer(*_lightBuffer, &lightData, sizeof(lightData));
		_commandContext.BindConstantBuffer(ShaderStage::Pixel, 1, _lightBuffer.get());
		_commandContext.UpdateConstantBuffer(*_shadowBuffer, &shadowData, sizeof(shadowData));
		_commandContext.BindConstantBuffer(ShaderStage::Pixel, 2, _shadowBuffer.get());

		_commandContext.BindRenderTarget(&sceneColor, nullptr);
		for (uint32 targetIndex = 0; targetIndex < GBufferTargetCount; ++targetIndex)
			_commandContext.BindShaderTexture(targetIndex, &gBuffer.GetTarget(static_cast<GBufferTarget>(targetIndex)));

		_commandContext.BindShaderTexture(GBufferTargetCount, &sceneDepth);
		_commandContext.BindShaderTexture(GBufferTargetCount + 1, spotLightCookieTexture);
		_commandContext.BindShaderTexture(GBufferTargetCount + 2, &cascadedShadowTexture);
		_commandContext.BindShaderTexture(GBufferTargetCount + 3, &screenSpaceAmbientOcclusion);
		_commandContext.BindSampler(0, &_gBufferSampler);
		_commandContext.BindSampler(1, &_spotCookieSampler);
		_commandContext.BindSampler(2, &_shadowSampler);
		_commandContext.BindSampler(3, &_ambientOcclusionSampler);
		_fullscreenRenderPass.Draw(*_pixelShader);
		_commandContext.UnbindShaderTextures(0, GBufferTargetCount + 4);
	}
}
