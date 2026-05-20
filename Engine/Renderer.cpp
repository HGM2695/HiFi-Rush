#include "Renderer.h"
#include "SpriteRenderPass.h"

namespace gm
{
	Renderer::Renderer(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory)
		: _spriteRenderPass(std::make_unique<SpriteRenderPass>(resources, commandContext, resourceFactory)) {}

	Renderer::~Renderer() = default;

	bool Renderer::Initialize()
	{
		return _spriteRenderPass->Initialize();
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
