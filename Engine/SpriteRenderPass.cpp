#include "SpriteRenderPass.h"
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

		Matrix CreateSpriteWorld(const SpriteRenderItem& item, const CameraViewInfo& viewInfo)
		{
			if (item.facingMode == SpriteFacingMode::None)
				return item.world;

			Vector3 scale{};
			Quaternion localRotation{};
			Vector3 position{};
			Matrix world = item.world;
			if (world.Decompose(scale, localRotation, position) == false)
				return item.world;

			if (item.facingMode == SpriteFacingMode::FixedUpAxisBillboard)
			{
				Vector3 fixedUp = Math::GetUpVector(localRotation);
				Vector3 cameraToSprite = position - viewInfo.position;
				Vector3 right = fixedUp.Cross(cameraToSprite);
				if (right.LengthSquared() <= 0.000001f)
					return item.world;

				fixedUp.Normalize();
				right.Normalize();
				Vector3 look = right.Cross(fixedUp);
				look.Normalize();
				return Matrix{
					right.x * scale.x, right.y * scale.x, right.z * scale.x, 0.f,
					fixedUp.x * scale.y, fixedUp.y * scale.y, fixedUp.z * scale.y, 0.f,
					look.x * scale.z, look.y * scale.z, look.z * scale.z, 0.f,
					position.x, position.y, position.z, 1.f
				};
			}

			if (item.facingMode == SpriteFacingMode::FixedRightAxisBillboard)
			{
				Vector3 fixedRight = Math::GetRightVector(localRotation);
				Vector3 cameraToSprite = position - viewInfo.position;
				Vector3 look = cameraToSprite - fixedRight * cameraToSprite.Dot(fixedRight);
				if (look.LengthSquared() <= 0.000001f)
					return item.world;

				fixedRight.Normalize();
				look.Normalize();
				Vector3 up = look.Cross(fixedRight);
				up.Normalize();
				return Matrix{
					fixedRight.x * scale.x, fixedRight.y * scale.x, fixedRight.z * scale.x, 0.f,
					up.x * scale.y, up.y * scale.y, up.z * scale.y, 0.f,
					look.x * scale.z, look.y * scale.z, look.z * scale.z, 0.f,
					position.x, position.y, position.z, 1.f
				};
			}

			Matrix facingRotation{};
			if (item.facingMode == SpriteFacingMode::Billboard)
			{
				return Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(viewInfo.rotation) * Matrix::CreateTranslation(position);
			}
			else
			{
				Vector3 facingDirection = viewInfo.position - position;
				facingDirection.y = 0.f;
				if (facingDirection.LengthSquared() <= 0.000001f)
					return item.world;

				facingDirection.Normalize();
				facingRotation = Math::CreateLookAtLH(Vector3::Zero, facingDirection, Vector3::Up).Invert();
			}

			return Matrix::CreateScale(scale) * facingRotation * Matrix::CreateTranslation(position);
		}
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

	void SpriteRenderPass::Prepare(const CameraViewInfo& viewInfo)
	{
		_constantBufferPool.ResetUsage();

		CameraConstantVS cameraConstantVS{};
		cameraConstantVS.view = viewInfo.view;
		cameraConstantVS.proj = viewInfo.projection;
		
		_cameraBuffer = _constantBufferPool.Acquire(sizeof(CameraConstantVS));
		_commandContext.UpdateConstantBuffer(*_cameraBuffer, &cameraConstantVS, sizeof(CameraConstantVS));

		for (SpriteRenderItem& item : _items)
		{
			item.cameraDepth = Vector3::Transform(Vector3::Transform(Vector3::Zero, item.world), viewInfo.view).z + item.sortDepthOffset;
			item.world = CreateSpriteWorld(item, viewInfo);
		}
	}

	void SpriteRenderPass::AppendTransparentRenderEntries(std::vector<TransparentRenderEntry>& entries) const
	{
		for (uint32 itemIndex = 0; itemIndex < _items.size(); ++itemIndex)
		{
			const SpriteRenderItem& item = _items[itemIndex];
			entries.push_back(TransparentRenderEntry{ TransparentRenderSource::Sprite, itemIndex, item.cameraDepth, item.submissionOrder });
		}
	}

	void SpriteRenderPass::Render(uint32 itemIndex)
	{
		GM_ASSERT_RETURN(itemIndex < _items.size(), "Sprite Render Item Index가 범위를 벗어났습니다.");
		const SpriteRenderItem& item = _items[itemIndex];
		if (item.material->GetVertexShader() == nullptr || item.material->GetPixelShader() == nullptr || item.material->GetTexture(TextureSlot::BaseColor) == nullptr)
			return;

		ObjectConstantVS objectConstantVS{};
		objectConstantVS.world = item.world;
		ConstantBuffer* objectBuffer = _constantBufferPool.Acquire(sizeof(ObjectConstantVS));
		_commandContext.UpdateConstantBuffer(*objectBuffer, &objectConstantVS, sizeof(ObjectConstantVS));
		_commandContext.BindMaterial(*item.material);
		_commandContext.BindMesh(*_unitQuadMesh);
		_commandContext.BindConstantBuffer(ShaderStage::Vertex, 0, objectBuffer);
		BindCameraConstant();
		BindMaterialConstantData(*item.material);
		_commandContext.DrawIndexed(_unitQuadMesh->GetIndexCount());
	}

	void SpriteRenderPass::Clear()
	{
		_items.clear();
		_cameraBuffer = nullptr;
	}

	void SpriteRenderPass::BindCameraConstant()
	{
		GM_ASSERT_RETURN(_cameraBuffer, "Sprite Camera ConstantBuffer가 준비되지 않았습니다.");
		_commandContext.BindConstantBuffer(ShaderStage::Vertex, 1, _cameraBuffer);
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
