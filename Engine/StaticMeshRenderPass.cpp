#include "StaticMeshRenderPass.h"
#include "ConstantBuffer.h"
#include "IGraphicsCommandContext.h"
#include "IGraphicsResourceFactory.h"
#include "Material.h"
#include "Mesh.h"
#include "StaticMesh.h"

namespace gm
{
	namespace
	{
		struct ObjectConstantVS
		{
			Matrix world;
		};

		struct CameraConstantVS
		{
			Matrix view;
			Matrix proj;
		};
	}

	StaticMeshRenderPass::StaticMeshRenderPass(
		Resources& resources,
		IGraphicsCommandContext& commandContext,
		IGraphicsResourceFactory& resourceFactory)
		: _resources(resources)
		, _commandContext(commandContext)
		, _resourceFactory(resourceFactory)
		, _constantBufferPool(resourceFactory)
	{
	}

	StaticMeshRenderPass::~StaticMeshRenderPass() = default;

	bool StaticMeshRenderPass::Initialize()
	{
		return true;
	}

	void StaticMeshRenderPass::Submit(const StaticMeshRenderItem& item)
	{
		if (item.staticMesh == nullptr)
			return;

		_items.push_back(item);
	}

	void StaticMeshRenderPass::Render(const CameraViewInfo& viewInfo)
	{
		_constantBufferPool.ResetUsage();

		CameraConstantVS cameraConstantVS{};
		cameraConstantVS.view = viewInfo.view;
		cameraConstantVS.proj = viewInfo.projection;

		ConstantBuffer* cameraBuffer = _constantBufferPool.Acquire(sizeof(CameraConstantVS));
		_commandContext.UpdateConstantBuffer(*cameraBuffer, &cameraConstantVS, sizeof(CameraConstantVS));
		_commandContext.BindConstantBuffer(ShaderStage::Vertex, 1, cameraBuffer);

		for (const StaticMeshRenderItem& item : _items)
		{
			const StaticMesh& staticMesh = *item.staticMesh;
			const std::shared_ptr<Mesh>& mesh = staticMesh.GetMesh();
			if (mesh == nullptr)
				continue;

			ObjectConstantVS objectConstantVS{};
			objectConstantVS.world = item.world;

			ConstantBuffer* objectBuffer = _constantBufferPool.Acquire(sizeof(ObjectConstantVS));
			_commandContext.UpdateConstantBuffer(*objectBuffer, &objectConstantVS, sizeof(ObjectConstantVS));
			_commandContext.BindConstantBuffer(ShaderStage::Vertex, 0, objectBuffer);
			_commandContext.BindMesh(*mesh);

			for (const MeshSection& section : staticMesh.GetSections())
			{
				if (section.indexCount == 0)
					continue;

				if (section.textureSetIndex >= item.materials.size())
					continue;

				const Material* material = item.materials[section.textureSetIndex];
				if (material == nullptr || material->GetVertexShader() == nullptr || material->GetPixelShader() == nullptr)
					continue;

				_commandContext.BindMaterial(*material);
				BindMaterialConstantData(*material);
				_commandContext.DrawIndexed(section.indexCount, section.indexStart, 0);
			}
		}

		Clear();
	}

	void StaticMeshRenderPass::Clear()
	{
		_items.clear();
	}

	void StaticMeshRenderPass::BindMaterialConstantData(const Material& material)
	{
		for (uint32 stageIndex = 0; stageIndex < ShaderStageCount; ++stageIndex)
		{
			const ShaderStage stage = static_cast<ShaderStage>(stageIndex);
			const Material::ConstantSlots& constantSlots = material.GetConstantSlots(stage);

			for (uint32 slot = 0; slot < MaxConstantBufferSlots; ++slot)
			{
				const Material::ConstantSlot& constantSlot = constantSlots[slot];
				if (constantSlot.IsValid() == false)
					continue;

				ConstantBuffer* buffer = _constantBufferPool.Acquire(constantSlot.Size());
				GM_ASSERT_RETURN(buffer, "Pool에서 Material ConstantBuffer를 가져오지 못했습니다.");

				_commandContext.UpdateConstantBuffer(*buffer, constantSlot.Data(), constantSlot.Size());
				_commandContext.BindConstantBuffer(stage, slot, buffer);
			}
		}
	}
}
