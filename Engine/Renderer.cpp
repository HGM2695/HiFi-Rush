#include "Renderer.h"
#include "SpriteRenderPass.h"
#include "UIRenderPass.h"

namespace gm
{
	Renderer::Renderer(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory)
		: _spriteRenderPass(std::make_unique<SpriteRenderPass>(resources, commandContext, resourceFactory))
		, _uiRenderPass(std::make_unique<UIRenderPass>(resources, commandContext, resourceFactory)) {}

	Renderer::~Renderer() = default;

	bool Renderer::Initialize()
	{
		GM_ASSERT_RETURN_VAL(_spriteRenderPass->Initialize(), false, "SpriteRenderPass 초기화 실패");
		GM_ASSERT_RETURN_VAL(_uiRenderPass->Initialize(), false, "UIRenderPass 초기화 실패");

		return true;
	}

	void Renderer::SubmitSprite(const SpriteRenderItem& item)
	{
		_spriteRenderPass->Submit(item);
	}

	void Renderer::SubmitStaticMesh(const StaticMeshRenderItem& item)
	{
		// StaticMesh 전용 RenderPass가 도입되면 이 경로에서 전달합니다.
	}

	void Renderer::SubmitUI(const UIRenderItem& item)
	{
		_uiRenderPass->Submit(item);
	}

	void Renderer::Render(const CameraViewInfo& viewInfo, uint32 width, uint32 height)
	{
		_spriteRenderPass->Render(viewInfo);
		_uiRenderPass->Render(width, height);
	}

	void Renderer::Clear()
	{
		_spriteRenderPass->Clear();
		_uiRenderPass->Clear();
	}
}
