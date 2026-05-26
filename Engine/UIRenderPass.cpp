#include "UIRenderPass.h"
#include "BuiltinGraphicsResources.h"
#include "ConstantBuffer.h"
#include "IGraphicsCommandContext.h"
#include "IGraphicsResourceFactory.h"
#include "Material.h"
#include "MathUtil.h"
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

		Vector2 ScreenToOrthoCenter(const Vector2& screenPosition, uint32 width, uint32 height)
		{
			return Vector2(
				screenPosition.x - static_cast<float>(width) * 0.5f,
				-screenPosition.y + static_cast<float>(height) * 0.5f
			);
		}

		Matrix CreateUIWorldMatrix(const Vector2& screenCenter, const Vector2& size, uint32 width, uint32 height)
		{
			const Vector2 orthoCenter = ScreenToOrthoCenter(screenCenter, width, height);
			return Math::CreateWorldMatrix(
				Vector3(orthoCenter.x, orthoCenter.y, 0.f),
				Math::IdentityQuaternion(),
				Vector3(size.x, size.y, 1.f)
			);
		}
	}

	UIRenderPass::UIRenderPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory)
		: _resources(resources)
		, _commandContext(commandContext)
		, _resourceFactory(resourceFactory)
		, _constantBufferPool(resourceFactory)
	{
	}

	UIRenderPass::~UIRenderPass() = default;

	bool UIRenderPass::Initialize()
	{
		_unitQuadMesh = _resources.Find<Mesh>(BuiltinResourceKey::UnitQuadMesh);
		GM_ASSERT_RETURN_VAL(_unitQuadMesh, false, "%ls 로드에 실패했습니다. BuiltinGraphics를 확인해주세요.", BuiltinResourceKey::UnitQuadMesh);

		return true;
	}

	void UIRenderPass::Submit(const UIRenderItem& item)
	{
		if (item.material == nullptr)
			return;

		_items.push_back(item);
	}

	void UIRenderPass::Render(uint32 width, uint32 height)
	{
		if (_items.empty())
			return;

		_constantBufferPool.ResetUsage();

		CameraConstantVS cameraConstantVS{};
		cameraConstantVS.view = Math::IdentityMatrix();
		cameraConstantVS.proj = Math::CreateOrthographicLH(static_cast<float>(width), static_cast<float>(height), 0.f, 1.f);

		ConstantBuffer* cameraBuffer = _constantBufferPool.Acquire(sizeof(CameraConstantVS));
		_commandContext.UpdateConstantBuffer(*cameraBuffer, &cameraConstantVS, sizeof(CameraConstantVS));
		_commandContext.BindConstantBuffer(ShaderStage::Vertex, 1, cameraBuffer);

		_commandContext.BindMesh(*_unitQuadMesh);

		for (const UIRenderItem& item : _items)
		{
			if (item.material->GetVertexShader() == nullptr || item.material->GetPixelShader() == nullptr)
				continue;

			ObjectConstantVS objectConstantVS{};
			objectConstantVS.world = CreateUIWorldMatrix(item.screenCenter, item.size, width, height);

			ConstantBuffer* objectBuffer = _constantBufferPool.Acquire(sizeof(ObjectConstantVS));
			GM_ASSERT_RETURN(objectBuffer, "UI ObjectConstantVS ConstantBuffer를 가져오지 못했습니다.");

			_commandContext.UpdateConstantBuffer(*objectBuffer, &objectConstantVS, sizeof(ObjectConstantVS));
			_commandContext.BindMaterial(*item.material);
			_commandContext.BindConstantBuffer(ShaderStage::Vertex, 0, objectBuffer);

			for (uint32 stageIndex = 0; stageIndex < ShaderStageCount; ++stageIndex)
			{
				const ShaderStage stage = static_cast<ShaderStage>(stageIndex);
				const Material::ConstantSlots& constantSlots = item.material->GetConstantSlots(stage);

				for (uint32 slot = 0; slot < MaxConstantBufferSlots; ++slot)
				{
					const Material::ConstantSlot& constantSlot = constantSlots[slot];
					if (constantSlot.IsValid() == false)
						continue;

					ConstantBuffer* buffer = _constantBufferPool.Acquire(constantSlot.Size());
					GM_ASSERT_RETURN(buffer, "UI Material ConstantBuffer를 가져오지 못했습니다.");

					_commandContext.UpdateConstantBuffer(*buffer, constantSlot.Data(), constantSlot.Size());
					_commandContext.BindConstantBuffer(stage, slot, buffer);
				}
			}

			_commandContext.DrawIndexed(_unitQuadMesh->GetIndexCount());
		}

		Clear();
	}

	void UIRenderPass::Clear()
	{
		_items.clear();
	}
}
