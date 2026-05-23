#pragma once

#include "EngineCore.h"

namespace gm
{
	class IGraphicsCommandContext;
	class IGraphicsResourceFactory;
	class Resources;
	class SpriteRenderPass;
	class UIRenderPass;

	struct SpriteRenderItem;
	struct ColorQuadRenderItem;
	struct TextureQuadRenderItem;
	struct CameraViewInfo;

	class Renderer
	{
	public:
		Renderer(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory);
		~Renderer();

		bool Initialize();
		void SubmitSprite(const SpriteRenderItem& item);
		void SubmitColorQuad(const ColorQuadRenderItem& item);
		void SubmitTextureQuad(const TextureQuadRenderItem& item);

		void Render(const CameraViewInfo& viewInfo, uint32 width, uint32 height);

		void Clear();

	private:
		std::unique_ptr<SpriteRenderPass> _spriteRenderPass;
		std::unique_ptr<UIRenderPass> _uiRenderPass;
	};
}
