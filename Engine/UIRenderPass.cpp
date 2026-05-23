#include "UIRenderPass.h"
#include "BuiltinGraphicsResources.h"
#include "ConstantBuffer.h"
#include "IGraphicsCommandContext.h"
#include "IGraphicsResourceFactory.h"
#include "MathUtil.h"
#include "Mesh.h"
#include "PipelineState.h"
#include "Resources.h"
#include "Sampler.h"
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

		struct ColorConstantPS
		{
			Color color;
		};

		struct TextureConstantPS
		{
			float textureLeft = 0.f;
			float textureTop = 0.f;
			float textureWidth = 1.f;
			float textureHeight = 1.f;
		};

		TextureConstantPS CreateTextureConstantPS(const TextureQuadRenderItem& item)
		{
			TextureConstantPS constant{};

			if (item.useSourceRect == false || item.texture == nullptr || item.texture->GetWidth() == 0 || item.texture->GetHeight() == 0)
				return constant;

			constant.textureLeft = static_cast<float>(item.sourceFrame.Left()) / static_cast<float>(item.texture->GetWidth());
			constant.textureTop = static_cast<float>(item.sourceFrame.Top()) / static_cast<float>(item.texture->GetHeight());
			constant.textureWidth = static_cast<float>(item.sourceFrame.Width()) / static_cast<float>(item.texture->GetWidth());
			constant.textureHeight = static_cast<float>(item.sourceFrame.Height()) / static_cast<float>(item.texture->GetHeight());

			return constant;
		}

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
		, _resourceFactory(resourceFactory) {}

	UIRenderPass::~UIRenderPass() = default;

	bool UIRenderPass::Initialize()
	{
		_unitQuadMesh = _resources.Find<Mesh>(BuiltinResourceKey::UnitQuadMesh);
		GM_ASSERT_RETURN_VAL(_unitQuadMesh, false, "%ls 로드에 실패했습니다. BuiltinGraphics를 확인해주세요.", BuiltinResourceKey::UnitQuadMesh);

		_solidColorPSO = _resources.Find<PipelineState>(BuiltinResourceKey::SolidColorPSO);
		GM_ASSERT_RETURN_VAL(_solidColorPSO, false, "%ls 로드에 실패했습니다. BuiltinGraphics를 확인해주세요.", BuiltinResourceKey::SolidColorPSO);

		_texturePSO = _resources.Find<PipelineState>(BuiltinResourceKey::SpriteTexturePSO);
		GM_ASSERT_RETURN_VAL(_texturePSO, false, "%ls 로드에 실패했습니다. BuiltinGraphics를 확인해주세요.", BuiltinResourceKey::SpriteTexturePSO);

		return CreateConstantBuffers();
	}

	void UIRenderPass::Submit(const ColorQuadRenderItem& item)
	{
		_colorItems.push_back(item);
	}

	void UIRenderPass::Submit(const TextureQuadRenderItem& item)
	{
		if (item.texture == nullptr)
			return;

		_textureItems.push_back(item);
	}

	void UIRenderPass::Render(uint32 width, uint32 height)
	{
		if (_colorItems.empty() && _textureItems.empty())
			return;

		CameraConstantVS cameraConstantVS{};
		cameraConstantVS.view = Math::IdentityMatrix();
		cameraConstantVS.proj = Math::CreateOrthographicLH(static_cast<float>(width), static_cast<float>(height), 0.f, 1.f);
		_commandContext.UpdateConstantBuffer(*_cameraConstantVS, &cameraConstantVS, sizeof(CameraConstantVS));
		_commandContext.SetConstantBuffer(ShaderStage::Vertex, 1, _cameraConstantVS.get());

		_commandContext.SetPipelineState(*_solidColorPSO);
		_commandContext.SetMesh(*_unitQuadMesh);

		for (const ColorQuadRenderItem& item : _colorItems)
		{
			ObjectConstantVS objectConstantVS{};
			objectConstantVS.world = CreateUIWorldMatrix(item.screenCenter, item.size, width, height);

			ColorConstantPS colorConstantPS{};
			colorConstantPS.color = item.color;

			_commandContext.UpdateConstantBuffer(*_objectConstantVS, &objectConstantVS, sizeof(ObjectConstantVS));
			_commandContext.UpdateConstantBuffer(*_colorConstantPS, &colorConstantPS, sizeof(ColorConstantPS));
			_commandContext.SetConstantBuffer(ShaderStage::Vertex, 0, _objectConstantVS.get());
			_commandContext.SetConstantBuffer(ShaderStage::Pixel, 0, _colorConstantPS.get());
			_commandContext.DrawIndexed(_unitQuadMesh->GetIndexCount());
		}

		_commandContext.SetPipelineState(*_texturePSO);
		_commandContext.SetMesh(*_unitQuadMesh);

		for (const TextureQuadRenderItem& item : _textureItems)
		{
			ObjectConstantVS objectConstantVS{};
			objectConstantVS.world = CreateUIWorldMatrix(item.screenCenter, item.size, width, height);

			TextureConstantPS textureConstantPS = CreateTextureConstantPS(item);

			_commandContext.UpdateConstantBuffer(*_objectConstantVS, &objectConstantVS, sizeof(ObjectConstantVS));
			_commandContext.UpdateConstantBuffer(*_textureConstantPS, &textureConstantPS, sizeof(TextureConstantPS));
			_commandContext.SetTexture(0, item.texture.get());
			_commandContext.SetSampler(0, item.sampler.get());
			_commandContext.SetConstantBuffer(ShaderStage::Vertex, 0, _objectConstantVS.get());
			_commandContext.SetConstantBuffer(ShaderStage::Pixel, 0, _textureConstantPS.get());
			_commandContext.DrawIndexed(_unitQuadMesh->GetIndexCount());
		}

		_commandContext.SetTexture(0, nullptr);
		_commandContext.SetSampler(0, nullptr);

		Clear();
	}

	void UIRenderPass::Clear()
	{
		_colorItems.clear();
		_textureItems.clear();
	}

	bool UIRenderPass::CreateConstantBuffers()
	{
		ConstantBufferDesc desc{};
		desc.size = sizeof(ObjectConstantVS);
		_objectConstantVS = _resourceFactory.CreateConstantBuffer(desc);
		GM_ASSERT_RETURN_VAL(_objectConstantVS, false, "_objectConstantVS 생성 실패");

		desc.size = sizeof(CameraConstantVS);
		_cameraConstantVS = _resourceFactory.CreateConstantBuffer(desc);
		GM_ASSERT_RETURN_VAL(_cameraConstantVS, false, "_cameraConstantVS 생성 실패");

		desc.size = sizeof(ColorConstantPS);
		_colorConstantPS = _resourceFactory.CreateConstantBuffer(desc);
		GM_ASSERT_RETURN_VAL(_colorConstantPS, false, "_colorConstantPS 생성 실패");

		desc.size = sizeof(TextureConstantPS);
		_textureConstantPS = _resourceFactory.CreateConstantBuffer(desc);
		GM_ASSERT_RETURN_VAL(_textureConstantPS, false, "_textureConstantPS 생성 실패");

		return true;
	}
}
