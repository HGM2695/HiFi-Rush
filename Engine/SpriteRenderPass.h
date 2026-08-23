#pragma once

#include "CameraViewInfo.h"
#include "ConstantBufferPool.h"
#include "RenderTypes.h"
#include <vector>

namespace gm
{
	class IGraphicsCommandContext;
	class IGraphicsResourceFactory;
	class Resources;
	class ConstantBuffer;
	class Material;

	class SpriteRenderPass
	{
	public:
		SpriteRenderPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory);
		~SpriteRenderPass();

		bool Initialize();
		void Submit(const SpriteRenderItem& item);
		void Prepare(const CameraViewInfo& viewInfo);
		void AppendTransparentRenderEntries(std::vector<TransparentRenderEntry>& entries) const;
		void Render(uint32 itemIndex);
		void Clear();

	private:
		void BindCameraConstant();
		void BindMaterialConstantData(const Material& material);

	private:
		Resources&							_resources;
		IGraphicsCommandContext&			_commandContext;
		IGraphicsResourceFactory&			_resourceFactory;

		std::shared_ptr<class Mesh>			_unitQuadMesh;

		std::vector<SpriteRenderItem>		_items;
		ConstantBufferPool					_constantBufferPool;
		ConstantBuffer*					_cameraBuffer = nullptr;
	};
}
