#pragma once

#include "CameraViewInfo.h"
#include "ConstantBufferPool.h"
#include "RenderTypes.h"
#include <vector>

namespace gm
{
	class Resources;
	class IGraphicsCommandContext;
	class IGraphicsResourceFactory;

	class Material;
	class ConstantBuffer;
	class Mesh;

	class StaticMeshRenderPass
	{
	public:
		StaticMeshRenderPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory);
		~StaticMeshRenderPass();

		bool Initialize();
		void Submit(const StaticMeshRenderItem& item);
		void Render(const CameraViewInfo& viewInfo, const BoundingFrustum* worldFrustum);
		void Clear();

	private:
		void BindMaterialConstantData(const Material& material);

	private:
		Resources&							_resources;
		IGraphicsCommandContext&			_commandContext;
		IGraphicsResourceFactory&			_resourceFactory;

		std::vector<StaticMeshRenderItem>	_items;
		ConstantBufferPool					_constantBufferPool;
	};
}
