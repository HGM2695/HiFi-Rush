#include "ToneMappingPass.h"
#include "BuiltinGraphicsResources.h"
#include "ConstantBuffer.h"
#include "FullscreenRenderPass.h"
#include "IGraphicsCommandContext.h"
#include "IGraphicsResourceFactory.h"
#include "Resources.h"
#include "Shader.h"
#include "Texture.h"
#include "ToneMappingTypes.h"

namespace gm
{
	namespace
	{
		struct ToneMappingConstantPS
		{
			float exposure = 0.f;
			Vector3 padding{};
		};

		static_assert(sizeof(ToneMappingConstantPS) % 16 == 0);
	}

	ToneMappingPass::ToneMappingPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory, FullscreenRenderPass& fullscreenRenderPass)
		: _resources(resources), _commandContext(commandContext), _resourceFactory(resourceFactory), _fullscreenRenderPass(fullscreenRenderPass)
	{
	}

	ToneMappingPass::~ToneMappingPass() = default;

	bool ToneMappingPass::Initialize()
	{
		_pixelShader = _resources.Find<Shader>(BuiltinResourceKey::ToneMappingPS);
		GM_ASSERT_RETURN_VAL(_pixelShader, false, "Tone Mapping Pixel Shader를 찾지 못했습니다.");

		ConstantBufferDesc desc{};
		desc.size = sizeof(ToneMappingConstantPS);
		_constantBuffer = _resourceFactory.CreateConstantBuffer(desc);
		GM_ASSERT_RETURN_VAL(_constantBuffer, false, "Tone Mapping Constant Buffer 생성에 실패했습니다.");
		return true;
	}

	void ToneMappingPass::Render(const Texture& sceneColor, const ToneMappingSettings& settings)
	{
		ToneMappingConstantPS constant{};
		constant.exposure = settings.exposure;
		_commandContext.UpdateConstantBuffer(*_constantBuffer, &constant, sizeof(constant));
		_commandContext.BindConstantBuffer(ShaderStage::Pixel, 0, _constantBuffer.get());
		_fullscreenRenderPass.Draw(sceneColor, *_pixelShader);
	}
}
