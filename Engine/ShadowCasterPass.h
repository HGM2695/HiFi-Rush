#pragma once

#include "ConstantBufferPool.h"
#include "RenderTypes.h"
#include <vector>

namespace gm
{
	class CascadedShadowMap;
	class IGraphicsCommandContext;
	class IGraphicsResourceFactory;
	class InstanceBuffer;
	class Material;
	class Mesh;
	class Resources;
	class Shader;
	struct MeshSection;

	class ShadowCasterPass
	{
	public:
		ShadowCasterPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory);
		~ShadowCasterPass();

		bool Initialize();
		void Submit(const StaticMeshRenderItem& item);
		void Submit(const SkeletalMeshRenderItem& item);
		void RenderCascade(CascadedShadowMap& shadowMap, uint32 cascadeIndex, const Matrix& lightViewProjection, const BoundingOrientedBox* casterBounds, bool isInstancingEnabled);
		void Clear();

	private:
		struct SkeletalShadowRenderItem
		{
			Matrix world = Matrix::CreateScale(1.f);
			const Mesh* mesh = nullptr;
			const MeshSection* section = nullptr;
			const std::vector<Matrix>* boneModelMatrices = nullptr;
			const Material* material = nullptr;
			size_t materialStateHash = 0;
		};

		void BuildRenderQueues(const BoundingOrientedBox* casterBounds);
		void RenderStaticBatch(const StaticMeshRenderBatch& batch, bool isMasked, bool isInstancingEnabled);
		void RenderStaticNormal(const StaticMeshRenderBatch& batch);
		bool RenderStaticInstanced(const StaticMeshRenderBatch& batch);
		void RenderSkeletal(const SkeletalShadowRenderItem& item, bool isMasked);
		void BindShadowMaterial(const Material& material, bool isMasked);
		bool EnsureInstanceBufferCapacity(uint32 requiredCapacity);

	private:
		Resources&								_resources;
		IGraphicsCommandContext&				_commandContext;
		IGraphicsResourceFactory&				_resourceFactory;
		std::vector<StaticMeshRenderItem>		_staticItems;
		std::vector<SkeletalMeshRenderItem>		_skeletalItems;
		std::vector<StaticMeshRenderBatch>		_opaqueStaticBatches;
		std::vector<StaticMeshRenderBatch>		_maskedStaticBatches;
		std::vector<SkeletalShadowRenderItem>	_opaqueSkeletalItems;
		std::vector<SkeletalShadowRenderItem>	_maskedSkeletalItems;
		ConstantBufferPool						_constantBufferPool;
		std::unique_ptr<InstanceBuffer>			_instanceBuffer;
		std::shared_ptr<Shader>					_staticVertexShader;
		std::shared_ptr<Shader>					_staticInstancedVertexShader;
		std::shared_ptr<Shader>					_skeletalVertexShader;
		std::shared_ptr<Shader>					_maskedPixelShader;
	};
}
