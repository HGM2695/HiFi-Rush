#pragma once

#include "EngineCore.h"

namespace gm
{
	class IGraphicsCommandContext;
	class IGraphicsResourceFactory;
	class Resources;
	class SpriteRenderPass;

	struct SpriteRenderItem;
	struct CameraViewInfo;

	class Renderer
	{
	public:
		Renderer(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory);
		~Renderer();

		bool Initialize();
		void SubmitSprite(const SpriteRenderItem& item);

		void Render(const CameraViewInfo& viewInfo);

		void Clear();

	private:
		std::unique_ptr<SpriteRenderPass> _spriteRenderPass;
	};
}
