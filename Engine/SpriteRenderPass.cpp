#include "SpriteRenderPass.h"
#include "BuiltinGraphicsResources.h"
#include "ConstantBuffer.h"
#include "IGraphicsCommandContext.h"
#include "IGraphicsResourceFactory.h"
#include "Material.h"
#include "Mesh.h"
#include "Resources.h"
#include "Texture.h"

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

	SpriteRenderPass::SpriteRenderPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory)
		: _resources(resources)
		, _commandContext(commandContext)
		, _resourceFactory(resourceFactory)
		, _constantBufferPool(resourceFactory)
	{
	}

	SpriteRenderPass::~SpriteRenderPass() = default;

	bool SpriteRenderPass::Initialize()
	{
		_unitQuadMesh = _resources.Find<Mesh>(BuiltinResourceKey::UnitQuadMesh);
		GM_ASSERT_RETURN_VAL(_unitQuadMesh, false, "%ls가 로드되지 않았습니다. BuiltinGraphics를 확인해주세요.", BuiltinResourceKey::UnitQuadMesh);

		return true;
	}

	void SpriteRenderPass::Submit(const SpriteRenderItem& item)
	{
		if (item.material == nullptr)
			return;

		_items.push_back(item);
	}

	void SpriteRenderPass::Render(const CameraViewInfo& viewInfo)
	{
		_constantBufferPool.ResetUsage();

		CameraConstantVS cameraConstantVS{};
		cameraConstantVS.view = viewInfo.view;
		cameraConstantVS.proj = viewInfo.projection;
		
		ConstantBuffer* cameraBuffer = _constantBufferPool.Acquire(sizeof(CameraConstantVS));
		_commandContext.UpdateConstantBuffer(*cameraBuffer, &cameraConstantVS, sizeof(CameraConstantVS));
		_commandContext.BindConstantBuffer(ShaderStage::Vertex, 1, cameraBuffer);

		for (const SpriteRenderItem& item : _items)
		{
			if (item.material->GetVertexShader() == nullptr || item.material->GetPixelShader() == nullptr
				|| item.material->GetTexture(TextureSlot::BaseColor) == nullptr)
				continue;

			ObjectConstantVS objectConstantVS{};
			objectConstantVS.world = item.world;

			ConstantBuffer* objectBuffer = _constantBufferPool.Acquire(sizeof(ObjectConstantVS));

			_commandContext.UpdateConstantBuffer(*objectBuffer, &objectConstantVS, sizeof(ObjectConstantVS));
			_commandContext.BindMaterial(*item.material);
			_commandContext.BindMesh(*_unitQuadMesh);
			_commandContext.BindConstantBuffer(ShaderStage::Vertex, 0, objectBuffer);
			BindMaterialConstantData(*item.material);
			_commandContext.DrawIndexed(_unitQuadMesh->GetIndexCount());
		}

		Clear();
	}

	void SpriteRenderPass::Clear()
	{
		_items.clear();
	}

	void SpriteRenderPass::BindMaterialConstantData(const Material& material)
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
