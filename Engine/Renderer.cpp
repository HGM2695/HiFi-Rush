#include "Renderer.h"
#include "GraphicsUtils.h"
#include "SpriteRenderPass.h"
#include "StaticMeshRenderPass.h"
#include "SkeletalMeshRenderPass.h"
#include "UIRenderPass.h"

#if GM_ENABLE_DEBUG_TOOLS
#include "IDebugRenderer.h"
#endif

namespace gm
{
	Renderer::Renderer(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory)
		: _spriteRenderPass(std::make_unique<SpriteRenderPass>(resources, commandContext, resourceFactory))
		, _staticMeshRenderPass(std::make_unique<StaticMeshRenderPass>(resources, commandContext, resourceFactory))
		, _skeletalMeshRenderPass(std::make_unique<SkeletalMeshRenderPass>(resources, commandContext, resourceFactory))
		, _uiRenderPass(std::make_unique<UIRenderPass>(resources, commandContext, resourceFactory))
	{
	}

	Renderer::~Renderer() = default;

	bool Renderer::Initialize()
	{
		GM_ASSERT_RETURN_VAL(_spriteRenderPass->Initialize(), false, "SpriteRenderPass 초기화에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(_staticMeshRenderPass->Initialize(), false, "StaticMeshRenderPass 초기화에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(_skeletalMeshRenderPass->Initialize(), false, "SkeletalMeshRenderPass 초기화에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(_uiRenderPass->Initialize(), false, "UIRenderPass 초기화에 실패했습니다.");

		return true;
	}

	void Renderer::SubmitSprite(const SpriteRenderItem& item)
	{
		_spriteRenderPass->Submit(item);
	}

	void Renderer::SubmitStaticMesh(const StaticMeshRenderItem& item)
	{
		_staticMeshRenderPass->Submit(item);
	}

	void Renderer::SubmitSkeletalMesh(const SkeletalMeshRenderItem& item)
	{
		_skeletalMeshRenderPass->Submit(item);
	}

	void Renderer::SubmitUI(const UIRenderItem& item)
	{
		_uiRenderPass->Submit(item);
	}

#if GM_ENABLE_DEBUG_TOOLS
	void Renderer::DebugDraw(IDebugRenderer& debugRenderer) const
	{
		if (_isBoundingVolumeDebugDrawEnabled == false)
			return;

		_staticMeshRenderPass->DebugDraw(debugRenderer);
		_skeletalMeshRenderPass->DebugDraw(debugRenderer);
	}
#endif

	void Renderer::Render(const CameraViewInfo& viewInfo, uint32 width, uint32 height)
	{
		const BoundingFrustum worldFrustum = CreateWorldFrustum(viewInfo.view, viewInfo.projection);

		_spriteRenderPass->Render(viewInfo);
		_staticMeshRenderPass->Render(viewInfo, &worldFrustum);
		_skeletalMeshRenderPass->Render(viewInfo, &worldFrustum);
		_uiRenderPass->Render(width, height);
	}

	void Renderer::Clear()
	{
		_spriteRenderPass->Clear();
		_staticMeshRenderPass->Clear();
		_skeletalMeshRenderPass->Clear();
		_uiRenderPass->Clear();
	}
}
