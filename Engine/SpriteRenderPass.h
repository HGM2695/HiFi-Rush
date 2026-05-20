#pragma once

#include "CameraViewInfo.h"
#include "RenderTypes.h"
#include <vector>

namespace gm
{
	class IGraphicsCommandContext;
	class IGraphicsResourceFactory;
	class Resources;
	class ConstantBuffer;

	class SpriteRenderPass
	{
	public:
		SpriteRenderPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory);
		~SpriteRenderPass();

		bool Initialize();
		void Submit(const SpriteRenderItem& item);
		void Render(const CameraViewInfo& viewInfo);
		void Clear();

	private:
		bool CreateConstantBuffers();

	private:
		Resources&							_resources;
		IGraphicsCommandContext&			_commandContext;
		IGraphicsResourceFactory&			_resourceFactory;
		std::shared_ptr<class Mesh>			_unitQuadMesh;
		std::vector<SpriteRenderItem>		_items;

		std::unique_ptr<ConstantBuffer>		_objectConstantVS;
		std::unique_ptr<ConstantBuffer>		_cameraConstantVS;
		std::unique_ptr<ConstantBuffer>		_objectConstantPS;
	};
}
