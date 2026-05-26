#pragma once

#include "ConstantBufferPool.h"
#include "RenderTypes.h"
#include <memory>
#include <vector>

namespace gm
{
	class ConstantBuffer;
	class IGraphicsCommandContext;
	class IGraphicsResourceFactory;
	class Material;
	class Mesh;
	class Resources;

	class UIRenderPass
	{
	public:
		UIRenderPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory);
		~UIRenderPass();

		bool Initialize();
		void Submit(const UIRenderItem& item);
		void Render(uint32 width, uint32 height);
		void Clear();

	private:
		Resources&							_resources;
		IGraphicsCommandContext&			_commandContext;
		IGraphicsResourceFactory&			_resourceFactory;

		std::shared_ptr<Mesh>				_unitQuadMesh;

		std::vector<UIRenderItem>			_items;
		ConstantBufferPool					_constantBufferPool;
	};
}
