#include "Renderer.h"
#include "SpriteRenderPass.h"

namespace gm
{
	Renderer::Renderer(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory)
		: _spriteRenderPass(std::make_unique<SpriteRenderPass>(resources, commandContext, resourceFactory)) {}

	Renderer::~Renderer() = default;

	bool Renderer::Initialize()
	{
		GM_ASSERT_RETURN_VAL(_spriteRenderPass->Initialize(), false, "SpriteRenderPass 초기화 실패");

		return true;
	}

	void Renderer::SubmitSprite(const SpriteRenderItem& item)
	{
		_spriteRenderPass->Submit(item);
	}

	void Renderer::Render(const CameraViewInfo& viewInfo)
	{
		_spriteRenderPass->Render(viewInfo);
	}

	void Renderer::Clear()
	{
		_spriteRenderPass->Clear();
	}
}
