#include "FXAARenderPass.h"
#include "BuiltinGraphicsResources.h"
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
		struct FXAAConstantPS
		{
			Vector2 texelSize{};
			float edgeThreshold = 0.125f;
			float minimumEdgeThreshold = 0.0312f;
			float subpixelBlending = 0.75f;
			Vector3 padding{};
		};

		static_assert(sizeof(FXAAConstantPS) % 16 == 0);
	}

	FXAARenderPass::FXAARenderPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory, FullscreenRenderPass& fullscreenRenderPass)
		: _resources(resources), _commandContext(commandContext), _resourceFactory(resourceFactory), _fullscreenRenderPass(fullscreenRenderPass)
	{
	}

	FXAARenderPass::~FXAARenderPass() = default;

	bool FXAARenderPass::Initialize()
	{
		_pixelShader = _resources.Find<Shader>(BuiltinResourceKey::FXAAPS);
		GM_ASSERT_RETURN_VAL(_pixelShader, false, "FXAA Pixel Shader를 찾지 못했습니다.");

		ConstantBufferDesc desc{};
		desc.size = sizeof(FXAAConstantPS);
		_constantBuffer = _resourceFactory.CreateConstantBuffer(desc);
		GM_ASSERT_RETURN_VAL(_constantBuffer, false, "FXAA Constant Buffer 생성에 실패했습니다.");
		return true;
	}

	void FXAARenderPass::Render(const Texture& sceneColor)
	{
		FXAAConstantPS constant{};
		constant.texelSize = Vector2{ 1.f / static_cast<float>(sceneColor.GetWidth()), 1.f / static_cast<float>(sceneColor.GetHeight()) };
		_commandContext.UpdateConstantBuffer(*_constantBuffer, &constant, sizeof(constant));
		_commandContext.BindConstantBuffer(ShaderStage::Pixel, 0, _constantBuffer.get());
		_fullscreenRenderPass.Draw(sceneColor, *_pixelShader);
	}
}
