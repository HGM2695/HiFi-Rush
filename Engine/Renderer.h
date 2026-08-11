#pragma once

#include "EngineCore.h"

namespace gm
{
	class IGraphicsCommandContext;
	class IGraphicsResourceFactory;
	class Resources;
	class SpriteRenderPass;
	class StaticMeshRenderPass;
	class SkeletalMeshRenderPass;
	class UIRenderPass;
#if GM_ENABLE_DEBUG_TOOLS
	class IDebugRenderer;

	struct MeshCullingDebugStats
	{
		uint32 submittedCount = 0;
		uint32 visibleCount = 0;
		uint32 culledCount = 0;
	};

	struct CullingDebugStats
	{
		MeshCullingDebugStats staticMesh{};
		MeshCullingDebugStats skeletalMesh{};
	};

	struct StaticMeshInstancingDebugStats
	{
		uint32 renderBatchCount = 0;
		uint32 normalDrawCallCount = 0;
		uint32 instancedDrawCallCount = 0;
		uint32 instancedInstanceCount = 0;
	};
#endif

	struct SpriteRenderItem;
	struct StaticMeshRenderItem;
	struct SkeletalMeshRenderItem;
	struct UIRenderItem;
	struct CameraViewInfo;

	class Renderer
	{
	public:
		Renderer(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory);
		~Renderer();

		bool Initialize();
		void SubmitSprite(const SpriteRenderItem& item);
		void SubmitStaticMesh(const StaticMeshRenderItem& item);
		void SubmitSkeletalMesh(const SkeletalMeshRenderItem& item);
		void SubmitUI(const UIRenderItem& item);

#if GM_ENABLE_DEBUG_TOOLS
		void SetBoundingVolumeDebugDrawEnabled(bool isEnabled) { _isBoundingVolumeDebugDrawEnabled = isEnabled; }
		bool IsBoundingVolumeDebugDrawEnabled() const { return _isBoundingVolumeDebugDrawEnabled; }
		void SetFrustumCullingEnabled(bool isEnabled) { _isFrustumCullingEnabled = isEnabled; }
		bool IsFrustumCullingEnabled() const { return _isFrustumCullingEnabled; }
		void SetStaticMeshInstancingEnabled(bool isEnabled) { _isStaticMeshInstancingEnabled = isEnabled; }
		bool IsStaticMeshInstancingEnabled() const { return _isStaticMeshInstancingEnabled; }
		CullingDebugStats GetCullingDebugStats() const;
		StaticMeshInstancingDebugStats GetStaticMeshInstancingDebugStats() const;
		void DebugDraw(IDebugRenderer& debugRenderer) const;
#endif

		void Render(const CameraViewInfo& viewInfo, uint32 width, uint32 height);

		void Clear();

	private:
		std::unique_ptr<SpriteRenderPass>	_spriteRenderPass;
		std::unique_ptr<StaticMeshRenderPass>	_staticMeshRenderPass;
		std::unique_ptr<SkeletalMeshRenderPass>	_skeletalMeshRenderPass;
		std::unique_ptr<UIRenderPass>		_uiRenderPass;

#if GM_ENABLE_DEBUG_TOOLS
		bool _isBoundingVolumeDebugDrawEnabled = false;
		bool _isFrustumCullingEnabled = true;
		bool _isStaticMeshInstancingEnabled = true;
#endif
	};
}
