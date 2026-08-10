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
#if GM_ENABLE_DEBUG_TOOLS
	class IDebugRenderer;
#endif

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
#if GM_ENABLE_DEBUG_TOOLS
		void DebugDraw(IDebugRenderer& debugRenderer) const;
		uint32 GetLastSubmittedItemCount() const { return _lastSubmittedItemCount; }
		uint32 GetLastVisibleItemCount() const { return _lastVisibleItemCount; }
		uint32 GetLastCulledItemCount() const { return _lastCulledItemCount; }
#endif
		void Render(const CameraViewInfo& viewInfo, const BoundingFrustum* worldFrustum);
		void Clear();

	private:
		void BuildRenderBatches(const BoundingFrustum* worldFrustum);
		void BindMaterialConstantData(const Material& material);

	private:
		Resources&							_resources;
		IGraphicsCommandContext&			_commandContext;
		IGraphicsResourceFactory&			_resourceFactory;

		std::vector<StaticMeshRenderItem>	_items;
		std::vector<StaticMeshRenderBatch>	_renderBatchList;
		ConstantBufferPool					_constantBufferPool;

#if GM_ENABLE_DEBUG_TOOLS
		uint32 _lastSubmittedItemCount = 0;
		uint32 _lastVisibleItemCount = 0;
		uint32 _lastCulledItemCount = 0;
#endif
	};
}
