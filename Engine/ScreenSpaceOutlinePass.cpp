#include "ScreenSpaceOutlinePass.h"
#include "BuiltinGraphicsResources.h"
#include "CameraViewInfo.h"
#include "ConstantBuffer.h"
#include "FullscreenRenderPass.h"
#include "IGraphicsCommandContext.h"
#include "IGraphicsResourceFactory.h"
#include "Resources.h"
#include "Shader.h"
#include "Texture.h"

namespace gm
{
	namespace
	{
		struct ScreenSpaceOutlineConstantPS
		{
			Matrix inverseViewProjection = Matrix::Identity;
			Matrix viewMatrix = Matrix::Identity;
			Vector2 texelSize{};
			float depthThreshold = 0.005f;
			float normalThreshold = 0.2f;
			Color outlineColor = Colors::Black;
			float thickness = 0.5f;
			uint32 debugOutput = 0;
			Vector2 padding{};
		};

		static_assert(sizeof(ScreenSpaceOutlineConstantPS) % 16 == 0);
	}

	ScreenSpaceOutlinePass::ScreenSpaceOutlinePass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory, FullscreenRenderPass& fullscreenRenderPass)
		: _resources(resources), _commandContext(commandContext), _resourceFactory(resourceFactory), _fullscreenRenderPass(fullscreenRenderPass)
	{
		_pointClampSampler.filter = TextureFilter::Point;
	}

	ScreenSpaceOutlinePass::~ScreenSpaceOutlinePass() = default;

	bool ScreenSpaceOutlinePass::Initialize(uint32 width, uint32 height)
	{
		_pixelShader = _resources.Find<Shader>(BuiltinResourceKey::ScreenSpaceOutlinePS);
		GM_ASSERT_RETURN_VAL(_pixelShader, false, "Screen Space Outline Pixel Shader를 찾지 못했습니다.");

		ConstantBufferDesc desc{};
		desc.size = sizeof(ScreenSpaceOutlineConstantPS);
		_constantBuffer = _resourceFactory.CreateConstantBuffer(desc);
		GM_ASSERT_RETURN_VAL(_constantBuffer, false, "Screen Space Outline Constant Buffer 생성에 실패했습니다.");

		TextureDesc textureDesc{};
		textureDesc.width = width;
		textureDesc.height = height;
		textureDesc.format = TextureFormat::R8UNorm;
		textureDesc.bindUsage = TextureBindUsage::RenderTarget | TextureBindUsage::ShaderResource;
		_debugTexture = _resourceFactory.CreateTexture(textureDesc);
		GM_ASSERT_RETURN_VAL(_debugTexture, false, "Screen Space Outline Debug Texture 생성에 실패했습니다.");
		return true;
	}

	void ScreenSpaceOutlinePass::Render(const Texture& sceneColor, const Texture& sceneDepth, const Texture& worldNormal, const Texture& materialData, Texture& output, const CameraViewInfo& viewInfo, uint32 width, uint32 height)
	{
		Draw(sceneColor, sceneDepth, worldNormal, materialData, output, viewInfo, width, height, false);
	}

	void ScreenSpaceOutlinePass::RenderDebug(const Texture& sceneColor, const Texture& sceneDepth, const Texture& worldNormal, const Texture& materialData, const CameraViewInfo& viewInfo, uint32 width, uint32 height)
	{
		Draw(sceneColor, sceneDepth, worldNormal, materialData, *_debugTexture, viewInfo, width, height, true);
	}

	void ScreenSpaceOutlinePass::Draw(const Texture& sceneColor, const Texture& sceneDepth, const Texture& worldNormal, const Texture& materialData, Texture& output, const CameraViewInfo& viewInfo, uint32 width, uint32 height, bool debugOutput)
	{
		ScreenSpaceOutlineConstantPS constant{};
		constant.inverseViewProjection = (viewInfo.view * viewInfo.projection).Invert();
		constant.viewMatrix = viewInfo.view;
		constant.texelSize = Vector2{ 1.f / static_cast<float>(width), 1.f / static_cast<float>(height) };
		constant.depthThreshold = _settings.depthThreshold;
		constant.normalThreshold = _settings.normalThreshold;
		constant.outlineColor = ConvertSRGBToLinear(_settings.color);
		constant.thickness = _settings.thickness;
		constant.debugOutput = debugOutput ? 1u : 0u;
		_commandContext.UpdateConstantBuffer(*_constantBuffer, &constant, sizeof(constant));
		_commandContext.BindConstantBuffer(ShaderStage::Pixel, 0, _constantBuffer.get());
		_commandContext.BindRenderTarget(&output, nullptr);
		_commandContext.BindShaderTexture(0, &sceneColor);
		_commandContext.BindShaderTexture(1, &sceneDepth);
		_commandContext.BindShaderTexture(2, &worldNormal);
		_commandContext.BindShaderTexture(3, &materialData);
		_commandContext.BindSampler(0, &_pointClampSampler);
		_fullscreenRenderPass.Draw(*_pixelShader);
		_commandContext.UnbindShaderTextures(0, 4);
	}
}
