#include "RenderTargetDebugPass.h"

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
		struct RenderTargetDebugConstantPS
		{
			Matrix inverseProjection = Matrix::Identity;
			uint32 view = 0;
			float valueScale = 1.f;
			Vector2 padding{};
		};
	}

	RenderTargetDebugPass::RenderTargetDebugPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory, FullscreenRenderPass& fullscreenRenderPass)
		: _resources(resources), _commandContext(commandContext), _resourceFactory(resourceFactory), _fullscreenRenderPass(fullscreenRenderPass)
	{
		_pointSamplerDesc.filter = TextureFilter::Point;
		_linearSamplerDesc.filter = TextureFilter::Linear;
	}

	RenderTargetDebugPass::~RenderTargetDebugPass() = default;

	bool RenderTargetDebugPass::Initialize()
	{
		_pixelShader = _resources.Find<Shader>(BuiltinResourceKey::RenderTargetDebugPS);
		GM_ASSERT_RETURN_VAL(_pixelShader, false, "Render Target Debug Pixel Shader를 찾지 못했습니다.");

		ConstantBufferDesc desc{};
		desc.size = sizeof(RenderTargetDebugConstantPS);
		_constantBuffer = _resourceFactory.CreateConstantBuffer(desc);
		GM_ASSERT_RETURN_VAL(_constantBuffer, false, "Render Target Debug Constant Buffer 생성에 실패했습니다.");
		return true;
	}

	void RenderTargetDebugPass::Render(const Texture& texture, RenderTargetDebugView view, const CameraViewInfo& viewInfo, float valueScale)
	{
		RenderTargetDebugConstantPS constant{};
		constant.inverseProjection = viewInfo.projection.Invert();
		constant.view = static_cast<uint32>(view);
		constant.valueScale = valueScale;
		_commandContext.UpdateConstantBuffer(*_constantBuffer, &constant, sizeof(constant));
		_commandContext.BindConstantBuffer(ShaderStage::Pixel, 0, _constantBuffer.get());
		_commandContext.BindShaderTexture(0, &texture);
		_commandContext.BindSampler(0, view == RenderTargetDebugView::BloomContribution ? &_linearSamplerDesc : &_pointSamplerDesc);
		_fullscreenRenderPass.Draw(*_pixelShader);
		_commandContext.UnbindShaderTextures(0, 1);
	}
}
