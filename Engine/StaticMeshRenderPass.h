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
	class InstanceBuffer;
	class Mesh;
	class Shader;

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
		uint32 GetLastRenderBatchCount() const { return _lastRenderBatchCount; }
		uint32 GetLastNormalDrawCallCount() const { return _lastNormalDrawCallCount; }
		uint32 GetLastInstancedDrawCallCount() const { return _lastInstancedDrawCallCount; }
		uint32 GetLastInstancedInstanceCount() const { return _lastInstancedInstanceCount; }
#endif
		void Render(const CameraViewInfo& viewInfo, const BoundingFrustum* worldFrustum, bool isInstancingEnabled);
		void Clear();

	private:
		void BuildRenderBatches(const BoundingFrustum* worldFrustum);
		bool CanRenderInstanced(const StaticMeshRenderBatch& batch, bool isInstancingEnabled) const;
		bool EnsureInstanceBufferCapacity(uint32 requiredCapacity);
		void RenderNormalBatch(const StaticMeshRenderBatch& batch);
		bool RenderInstancedBatch(const StaticMeshRenderBatch& batch);
		void BindMaterialConstantData(const Material& material);

	private:
		Resources&							_resources;
		IGraphicsCommandContext&			_commandContext;
		IGraphicsResourceFactory&			_resourceFactory;

		std::vector<StaticMeshRenderItem>	_items;
		std::vector<StaticMeshRenderBatch>	_renderBatchList;
		ConstantBufferPool					_constantBufferPool;
		std::unique_ptr<InstanceBuffer>		_instanceBuffer;
		std::shared_ptr<Shader>				_staticMeshVertexShader;
		std::shared_ptr<Shader>				_staticMeshInstancedVertexShader;

#if GM_ENABLE_DEBUG_TOOLS
		uint32 _lastSubmittedItemCount = 0;
		uint32 _lastVisibleItemCount = 0;
		uint32 _lastCulledItemCount = 0;
		uint32 _lastRenderBatchCount = 0;
		uint32 _lastNormalDrawCallCount = 0;
		uint32 _lastInstancedDrawCallCount = 0;
		uint32 _lastInstancedInstanceCount = 0;
#endif
	};
}
