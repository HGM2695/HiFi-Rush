#pragma once

#include "RenderTypes.h"
#include <memory>
#include <vector>

namespace gm
{
	class ConstantBuffer;
	class IGraphicsCommandContext;
	class IGraphicsResourceFactory;
	class Mesh;
	class PipelineState;
	class Resources;

	class UIRenderPass
	{
	public:
		UIRenderPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory);
		~UIRenderPass();

		bool Initialize();
		void Submit(const ColorQuadRenderItem& item);
		void Submit(const TextureQuadRenderItem& item);
		void Render(uint32 width, uint32 height);
		void Clear();

	private:
		bool CreateConstantBuffers();

	private:
		Resources&							_resources;
		IGraphicsCommandContext&			_commandContext;
		IGraphicsResourceFactory&			_resourceFactory;
		std::shared_ptr<Mesh>				_unitQuadMesh;
		std::shared_ptr<PipelineState>		_solidColorPSO;
		std::shared_ptr<PipelineState>		_texturePSO;
		std::vector<ColorQuadRenderItem>	_colorItems;
		std::vector<TextureQuadRenderItem>	_textureItems;

		std::unique_ptr<ConstantBuffer>		_objectConstantVS;
		std::unique_ptr<ConstantBuffer>		_cameraConstantVS;
		std::unique_ptr<ConstantBuffer>		_colorConstantPS;
		std::unique_ptr<ConstantBuffer>		_textureConstantPS;
	};
}
