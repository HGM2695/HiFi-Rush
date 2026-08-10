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

	class SkeletalMeshRenderPass
	{
	public:
		SkeletalMeshRenderPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory);
		~SkeletalMeshRenderPass();

		bool Initialize();
		void Submit(const SkeletalMeshRenderItem& item);
		void Render(const CameraViewInfo& viewInfo, const BoundingFrustum* worldFrustum);
		void Clear();

	private:
		void BindMaterialConstantData(const Material& material);

	private:
		Resources&							_resources;
		IGraphicsCommandContext&			_commandContext;
		IGraphicsResourceFactory&			_resourceFactory;

		std::vector<SkeletalMeshRenderItem>	_items;
		ConstantBufferPool					_constantBufferPool;
	};
}
