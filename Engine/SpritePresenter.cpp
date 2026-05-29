#include "SpritePresenter.h"
#include "Application.h"
#include "BuiltinGraphicsResources.h"
#include "Material.h"
#include "Renderer.h"
#include "RenderTypes.h"
#include "Resources.h"
#include "Shader.h"
#include "SpriteFrame.h"
#include "Texture.h"

namespace gm
{
	SpritePresenter::SpritePresenter() = default;
	SpritePresenter::~SpritePresenter() = default;

	void SpritePresenter::EnsureDefaultMaterial()
	{
		if (_material)
			return;

		Resources& resources = APPLICATION.GetResources();

		_material = std::make_unique<Material>(
			Material::MaterialBuilder(resources)
				.SetVertexShader(BuiltinResourceKey::QuadVS)
				.SetPixelShader(BuiltinResourceKey::SpriteTexturePS)
				.SetSamplerFilter(TextureSlot::BaseColor, TextureFilter::Point)
				.SetCullMode(CullMode::None)
				.SetDepthEnable(true)
				.SetDepthWriteEnable(true)
				.SetBlendEnable(true)
				.Build()
		);

		GM_ASSERT_RETURN(_material->GetVertexShader(), "Sprite 기본 VertexShader가 로드되지 않았습니다.");
		GM_ASSERT_RETURN(_material->GetPixelShader(), "Sprite 기본 PixelShader가 로드되지 않았습니다.");

		UpdateSpriteConstantData();
	}

	void SpritePresenter::Submit(const Matrix& world) const
	{
		if (_material == nullptr || _material->GetVertexShader() == nullptr || _material->GetPixelShader() == nullptr
			|| _material->GetTexture(TextureSlot::BaseColor) == nullptr)
			return;

		SpriteRenderItem item{};
		item.world = world;
		item.material = _material.get();

		APPLICATION.GetRenderer().SubmitSprite(item);
	}

	void SpritePresenter::SetTexture(const std::shared_ptr<Texture>& texture, TextureSlot slot)
	{
		GM_ASSERT_RETURN(texture, "texture가 nullptr입니다.");

		EnsureDefaultMaterial();
		_material->SetTexture(slot, texture);

		if (slot == TextureSlot::BaseColor)
			UpdateSpriteConstantData();
	}

	void SpritePresenter::SetSamplerDesc(const SamplerDesc& desc, TextureSlot slot)
	{
		EnsureDefaultMaterial();
		_material->SetSamplerDesc(slot, desc);
	}

	void SpritePresenter::SetMaterial(const Material& material)
	{
		_material = std::make_unique<Material>(material);
		UpdateSpriteConstantData();
	}

	void SpritePresenter::SetSourceRect(const Rect& rect)
	{
		_sourceRect = rect;
		_useSourceRect = true;
		UpdateSpriteConstantData();
	}

	void SpritePresenter::SetSourceRect(const SpriteFrame& frame)
	{
		SetSourceRect(Rect
		{
			static_cast<float>(frame.Left()),
			static_cast<float>(frame.Top()),
			static_cast<float>(frame.Width()),
			static_cast<float>(frame.Height())
		});
	}

	void SpritePresenter::DisableSourceRect()
	{
		_useSourceRect = false;
		UpdateSpriteConstantData();
	}

	std::shared_ptr<Texture> SpritePresenter::GetTexture(TextureSlot slot) const
	{
		if (_material == nullptr)
			return nullptr;

		return _material->GetTexture(slot);
	}

	const SamplerDesc& SpritePresenter::GetSamplerDesc(TextureSlot slot) const
	{
		static const SamplerDesc defaultDesc{};

		if (_material == nullptr)
			return defaultDesc;

		return _material->GetSamplerDesc(slot);
	}

	Material* SpritePresenter::GetMaterial()
	{
		EnsureDefaultMaterial();
		return _material.get();
	}

	void SpritePresenter::UpdateSpriteConstantData()
	{
		if (_material == nullptr)
			return;

		const Rect uvRect = CreateUVRect();
		SpriteConstantPS constant{};
		constant.textureLeft = uvRect.left;
		constant.textureTop = uvRect.top;
		constant.textureWidth = uvRect.width;
		constant.textureHeight = uvRect.height;

		_material->SetConstantData(ShaderStage::Pixel, 0, constant);
	}

	Rect SpritePresenter::CreateUVRect() const
	{
		if (_useSourceRect == false)
			return Rect{ 0.f, 0.f, 1.f, 1.f };

		const std::shared_ptr<Texture> texture = _material ? _material->GetTexture(TextureSlot::BaseColor) : nullptr;
		if (texture == nullptr || texture->GetWidth() == 0 || texture->GetHeight() == 0)
			return Rect{ 0.f, 0.f, 1.f, 1.f };

		return Rect
		{
			_sourceRect.left / static_cast<float>(texture->GetWidth()),
			_sourceRect.top / static_cast<float>(texture->GetHeight()),
			_sourceRect.width / static_cast<float>(texture->GetWidth()),
			_sourceRect.height / static_cast<float>(texture->GetHeight())
		};
	}
}
