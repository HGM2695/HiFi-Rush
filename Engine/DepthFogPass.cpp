#include "DepthFogPass.h"
#include "BuiltinGraphicsResources.h"
#include "CameraViewInfo.h"
#include "ConstantBuffer.h"
#include "FogTypes.h"
#include "FullscreenRenderPass.h"
#include "GraphicsUtils.h"
#include "IGraphicsCommandContext.h"
#include "IGraphicsResourceFactory.h"
#include "Resources.h"
#include "Shader.h"
#include "Texture.h"

namespace gm
{
	namespace
	{
		struct DepthFogCameraConstantPS
		{
			Matrix inverseProjection = Matrix::Identity;
		};

		struct DepthFogConstantPS
		{
			Color fogColor = Colors::Black;
			float startDistance = 0.f;
			float endDistance = 0.f;
			float density = 0.f;
			uint32 enabled = 0;
		};

		static_assert(sizeof(DepthFogCameraConstantPS) % 16 == 0);
		static_assert(sizeof(DepthFogConstantPS) % 16 == 0);
	}

	DepthFogPass::DepthFogPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory, FullscreenRenderPass& fullscreenRenderPass)
		: _resources(resources), _commandContext(commandContext), _resourceFactory(resourceFactory), _fullscreenRenderPass(fullscreenRenderPass)
	{
		_depthSampler.filter = TextureFilter::Point;
	}

	DepthFogPass::~DepthFogPass() = default;

	bool DepthFogPass::Initialize()
	{
		_pixelShader = _resources.Find<Shader>(BuiltinResourceKey::DepthFogPS);
		GM_ASSERT_RETURN_VAL(_pixelShader, false, "Depth Fog Pixel Shader를 찾지 못했습니다.");

		ConstantBufferDesc desc{};
		desc.size = sizeof(DepthFogCameraConstantPS);
		_cameraBuffer = _resourceFactory.CreateConstantBuffer(desc);
		GM_ASSERT_RETURN_VAL(_cameraBuffer, false, "Depth Fog Camera Constant Buffer 생성에 실패했습니다.");

		desc.size = sizeof(DepthFogConstantPS);
		_fogBuffer = _resourceFactory.CreateConstantBuffer(desc);
		GM_ASSERT_RETURN_VAL(_fogBuffer, false, "Depth Fog Constant Buffer 생성에 실패했습니다.");
		return true;
	}

	void DepthFogPass::Render(const Texture& sceneColor, const Texture& sceneDepth, Texture& output, const CameraViewInfo& viewInfo, const DepthFogSettings& settings)
	{
		DepthFogCameraConstantPS cameraConstant{};
		cameraConstant.inverseProjection = viewInfo.projection.Invert();
		_commandContext.UpdateConstantBuffer(*_cameraBuffer, &cameraConstant, sizeof(cameraConstant));
		_commandContext.BindConstantBuffer(ShaderStage::Pixel, 0, _cameraBuffer.get());
		BindFogConstants(settings);

		_commandContext.BindRenderTarget(&output, nullptr);
		_commandContext.BindShaderTexture(0, &sceneColor);
		_commandContext.BindShaderTexture(1, &sceneDepth);
		_commandContext.BindSampler(0, &_sceneColorSampler);
		_commandContext.BindSampler(1, &_depthSampler);
		_fullscreenRenderPass.Draw(*_pixelShader);
		_commandContext.UnbindShaderTextures(0, 2);
	}

	void DepthFogPass::BindForwardConstants(const DepthFogSettings& settings)
	{
		BindFogConstants(settings);
	}

	void DepthFogPass::BindFogConstants(const DepthFogSettings& settings)
	{
		DepthFogConstantPS fogConstant{};
		fogConstant.fogColor = ConvertSRGBToLinear(settings.fogColor);
		fogConstant.startDistance = settings.startDistance;
		fogConstant.endDistance = settings.endDistance;
		fogConstant.density = settings.density;
		fogConstant.enabled = settings.enabled ? 1u : 0u;
		_commandContext.UpdateConstantBuffer(*_fogBuffer, &fogConstant, sizeof(fogConstant));
		_commandContext.BindConstantBuffer(ShaderStage::Pixel, 1, _fogBuffer.get());
	}
}
