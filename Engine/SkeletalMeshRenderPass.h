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
	class ConstantBuffer;
	class Material;
	class Mesh;
	class Shader;
	struct MeshSection;

	class SkeletalMeshRenderPass
	{
	public:
		SkeletalMeshRenderPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory);
		~SkeletalMeshRenderPass();

		bool Initialize();
		void Submit(const SkeletalMeshRenderItem& item);
#if GM_ENABLE_DEBUG_TOOLS
		void DebugDraw(IDebugRenderer& debugRenderer) const;
		uint32 GetLastSubmittedItemCount() const { return _lastSubmittedItemCount; }
		uint32 GetLastVisibleItemCount() const { return _lastVisibleItemCount; }
		uint32 GetLastCulledItemCount() const { return _lastCulledItemCount; }
#endif
		void Prepare(const CameraViewInfo& viewInfo, const BoundingFrustum* worldFrustum);
		void RenderOpaqueAndMasked();
		void AppendTransparentRenderEntries(std::vector<TransparentRenderEntry>& entries) const;
		void RenderTransparent(uint32 itemIndex);
		void Clear();

	private:
		struct SectionRenderItem
		{
			Matrix						world = Matrix::CreateScale(1.f);
			const Mesh*					mesh = nullptr;
			const MeshSection*			section = nullptr;
			const std::vector<Matrix>*	boneModelMatrices = nullptr;
			const Material*				material = nullptr;
			size_t						materialStateHash = 0;
			float						cameraDepth = 0.f;
			uint64						submissionOrder = 0;
		};

		void BuildRenderQueues(const Matrix& view, const BoundingFrustum* worldFrustum);
		void BindCameraConstant();
		void RenderSection(const SectionRenderItem& item, bool isGBufferPass);
		void BindMaterialSurfaceConstant(const Material& material);
		void BindMaterialConstantData(const Material& material);

	private:
		Resources&							_resources;
		IGraphicsCommandContext&			_commandContext;
		IGraphicsResourceFactory&			_resourceFactory;

		std::vector<SkeletalMeshRenderItem>	_items;
		std::vector<SectionRenderItem>		_opaqueRenderItems;
		std::vector<SectionRenderItem>		_maskedRenderItems;
		std::vector<SectionRenderItem>		_transparentRenderItems;
		ConstantBufferPool					_constantBufferPool;
		ConstantBuffer*					_cameraBuffer = nullptr;
		std::shared_ptr<Shader>				_skeletalMeshVertexShader;
		std::shared_ptr<Shader>				_gBufferPixelShader;

#if GM_ENABLE_DEBUG_TOOLS
		uint32 _lastSubmittedItemCount = 0;
		uint32 _lastVisibleItemCount = 0;
		uint32 _lastCulledItemCount = 0;
#endif
	};
}
