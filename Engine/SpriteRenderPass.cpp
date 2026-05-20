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

		struct SpriteConstantPS
		{
			float textureLeft = 0.f;
			float textureTop = 0.f;
			float textureWidth = 1.f;
			float textureHeight = 1.f;
		};

		SpriteConstantPS CreateSpriteConstantPS(const SpriteRenderItem& item)
		{
			SpriteConstantPS constant{};
			const std::shared_ptr<Texture> texture = item.material->GetTexture(0);

			if (item.useSourceRect == false || texture == nullptr || texture->GetWidth() == 0 || texture->GetHeight() == 0)
				return constant;

			constant.textureLeft = static_cast<float>(item.sourceFrame.left) / static_cast<float>(texture->GetWidth());
			constant.textureTop = static_cast<float>(item.sourceFrame.top) / static_cast<float>(texture->GetHeight());
			constant.textureWidth = static_cast<float>(item.sourceFrame.width) / static_cast<float>(texture->GetWidth());
			constant.textureHeight = static_cast<float>(item.sourceFrame.height) / static_cast<float>(texture->GetHeight());

			return constant;
		}
	}

	SpriteRenderPass::SpriteRenderPass(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory)
		: _resources(resources)
		, _commandContext(commandContext)
		, _resourceFactory(resourceFactory) {}

	SpriteRenderPass::~SpriteRenderPass() = default;

	bool SpriteRenderPass::Initialize()
	{
		_unitQuadMesh = _resources.Find<Mesh>(BuiltinResourceKey::UnitQuadMesh);
		GM_ASSERT_RETURN_VAL(_unitQuadMesh, false, "%ls 가 로드되지 않았습니다. BuiltinGraphics를 확인해주세요", BuiltinResourceKey::UnitQuadMesh);

		return CreateConstantBuffers();
	}

	void SpriteRenderPass::Submit(const SpriteRenderItem& item)
	{
		if (item.material == nullptr)
			return;

		_items.push_back(item);
	}

	void SpriteRenderPass::Render(const CameraViewInfo& viewInfo)
	{
		CameraConstantVS cameraConstantVS{};
		cameraConstantVS.view = viewInfo.view;
		cameraConstantVS.proj = viewInfo.projection;
		_commandContext.UpdateConstantBuffer(*_cameraConstantVS, &cameraConstantVS, sizeof(CameraConstantVS));
		_commandContext.SetConstantBuffer(ShaderStage::Vertex, 1, _cameraConstantVS.get());

		for (const SpriteRenderItem& item : _items)
		{
			if (item.material->GetPipelineState() == nullptr || item.material->GetTexture(0) == nullptr || item.material->GetSampler(0) == nullptr)
				continue;

			ObjectConstantVS objectConstantVS{};
			objectConstantVS.world = item.world;

			SpriteConstantPS objectConstantPS = CreateSpriteConstantPS(item);

			_commandContext.UpdateConstantBuffer(*_objectConstantVS, &objectConstantVS, sizeof(ObjectConstantVS));
			_commandContext.UpdateConstantBuffer(*_objectConstantPS, &objectConstantPS, sizeof(SpriteConstantPS));
			_commandContext.SetMaterial(*item.material);
			_commandContext.SetMesh(*_unitQuadMesh);
			_commandContext.SetConstantBuffer(ShaderStage::Vertex, 0, _objectConstantVS.get());
			_commandContext.SetConstantBuffer(ShaderStage::Pixel, 0, _objectConstantPS.get());
			_commandContext.DrawIndexed(_unitQuadMesh->GetIndexCount());
		}

		Clear();
	}

	void SpriteRenderPass::Clear()
	{
		_items.clear();
	}

	bool SpriteRenderPass::CreateConstantBuffers()
	{
		ConstantBufferDesc desc{};
		desc.size = sizeof(ObjectConstantVS);
		_objectConstantVS = _resourceFactory.CreateConstantBuffer(desc);
		GM_ASSERT_RETURN_VAL(_objectConstantVS, false, "_objectConstantVS 생성 실패");

		desc.size = sizeof(CameraConstantVS);
		_cameraConstantVS = _resourceFactory.CreateConstantBuffer(desc);
		GM_ASSERT_RETURN_VAL(_cameraConstantVS, false, "_cameraConstantVS 생성 실패");

		desc.size = sizeof(SpriteConstantPS);
		_objectConstantPS = _resourceFactory.CreateConstantBuffer(desc);
		GM_ASSERT_RETURN_VAL(_objectConstantPS, false, "_objectConstantPS 생성 실패");

		return true;
	}
}
