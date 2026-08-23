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

#include <algorithm>
#include <array>

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
				.SetShadingModel(ShadingModel::Unlit)
				.SetSurfaceMode(SurfaceMode::Transparent)
				.SetVertexShader(BuiltinResourceKey::QuadVS)
				.SetPixelShader(BuiltinResourceKey::SpriteTexturePS)
				.SetSamplerFilter(TextureSlot::BaseColor, TextureFilter::Point)
				.SetCullMode(CullMode::None)
				.SetDepthEnable(true)
				.SetDepthWriteEnable(false)
				.SetBlendEnable(true)
				.Build()
		);

		GM_ASSERT_RETURN(_material->GetVertexShader(), "Sprite 기본 VertexShader가 로드되지 않았습니다.");
		GM_ASSERT_RETURN(_material->GetPixelShader(), "Sprite 기본 PixelShader가 로드되지 않았습니다.");

		UpdateSpriteConstantData();
	}

	void SpritePresenter::EnableEffectRendering()
	{
		if (_isEffectRenderingEnabled)
			return;

		std::array<std::shared_ptr<Texture>, TextureSlotCount> textures{};
		std::array<SamplerDesc, TextureSlotCount> samplers{};
		if (_material)
		{
			for (uint32 slotIndex = 0; slotIndex < TextureSlotCount; ++slotIndex)
			{
				const TextureSlot slot = ToTextureSlot(slotIndex);
				textures[slotIndex] = _material->GetTexture(slot);
				samplers[slotIndex] = _material->GetSamplerDesc(slot);
			}
		}

		_material = std::make_unique<Material>(
			Material::MaterialBuilder(APPLICATION.GetResources())
				.SetShadingModel(ShadingModel::Unlit)
				.SetSurfaceMode(SurfaceMode::Transparent)
				.SetOutlineMode(OutlineMode::Disabled)
				.SetVertexShader(BuiltinResourceKey::QuadVS)
				.SetPixelShader(BuiltinResourceKey::EffectSpritePS)
				.SetSamplerFilter(TextureSlot::BaseColor, TextureFilter::Linear)
				.SetSamplerFilter(TextureSlot::Custom0, TextureFilter::Linear)
				.SetCullMode(CullMode::None)
				.SetDepthEnable(true)
				.SetDepthWriteEnable(false)
				.SetBlendEnable(true)
				.Build()
		);

		for (uint32 slotIndex = 0; slotIndex < TextureSlotCount; ++slotIndex)
		{
			if (textures[slotIndex] == nullptr)
				continue;

			const TextureSlot slot = ToTextureSlot(slotIndex);
			_material->SetTexture(slot, textures[slotIndex]);
			_material->SetSamplerDesc(slot, samplers[slotIndex]);
		}

		_isEffectRenderingEnabled = true;
		UpdateSpriteConstantData();
		UpdateEffectConstantData();
	}

	void SpritePresenter::Submit(const Matrix& world) const
	{
		if (_material == nullptr || _material->GetVertexShader() == nullptr || _material->GetPixelShader() == nullptr
			|| _material->GetTexture(TextureSlot::BaseColor) == nullptr)
			return;

		SpriteRenderItem item{};
		item.world = world;
		item.material = _material.get();
		item.facingMode = _facingMode;
		item.sortDepthOffset = _sortDepthOffset;

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

	void SpritePresenter::SetFacingMode(SpriteFacingMode facingMode)
	{
		GM_ASSERT_RETURN(facingMode < SpriteFacingMode::Count, "지원하지 않는 Sprite Facing Mode입니다.");
		_facingMode = facingMode;
	}

	void SpritePresenter::SetOpacity(float opacity)
	{
		_opacity = std::clamp(opacity, 0.f, 1.f);
		UpdateSpriteConstantData();
	}

	void SpritePresenter::SetFillRatio(float ratio)
	{
		_fillRatio = std::clamp(ratio, 0.f, 1.f);
		UpdateSpriteConstantData();
	}

	void SpritePresenter::SetRadialFill(const Vector2& center, float startAngle, float sweepAngle)
	{
		GM_ASSERT_RETURN(sweepAngle >= 0.f, "Radial Fill의 Sweep Angle은 0 이상이어야 합니다.");
		_fillMode = 1;
		_radialCenter = center;
		_radialStartAngle = startAngle;
		_radialSweepAngle = sweepAngle;
		UpdateSpriteConstantData();
	}

	void SpritePresenter::SetDissolveTexture(const std::shared_ptr<Texture>& texture)
	{
		GM_ASSERT_RETURN(texture, "Effect Dissolve Texture가 nullptr입니다.");
		EnableEffectRendering();
		_material->SetTexture(TextureSlot::Custom0, texture);
		_effectConstant.dissolveEnabled = 1;
		UpdateEffectConstantData();
	}

	void SpritePresenter::SetDissolveThreshold(float threshold)
	{
		EnableEffectRendering();
		_effectConstant.dissolveThreshold = threshold;
		UpdateEffectConstantData();
	}

	void SpritePresenter::DisableDissolve()
	{
		_effectConstant.dissolveEnabled = 0;
		UpdateEffectConstantData();
	}

	void SpritePresenter::SetEffectEmissive(const Color& color, float intensity)
	{
		GM_ASSERT_RETURN(intensity >= 0.f, "Effect Emissive Intensity는 0 이상이어야 합니다.");
		EnableEffectRendering();
		_effectConstant.emissiveColor = color;
		_effectConstant.emissiveIntensity = intensity;
		UpdateEffectConstantData();
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
		constant.uvOffset = Vector2{ uvRect.left, uvRect.top };
		constant.uvScale = Vector2{ uvRect.width, uvRect.height };
		constant.opacity = _opacity;
		constant.fillRatio = _fillRatio;
		constant.fillMode = _fillMode;
		constant.radialStartAngle = _radialStartAngle;
		constant.radialSweepAngle = _radialSweepAngle;
		constant.radialCenter = _radialCenter;

		_material->SetConstantData(ShaderStage::Pixel, 0, constant);
	}

	void SpritePresenter::UpdateEffectConstantData()
	{
		if (_material == nullptr || _isEffectRenderingEnabled == false)
			return;

		EffectMaterialConstantPS constant = _effectConstant;
		constant.emissiveColor = ConvertSRGBToLinear(constant.emissiveColor);
		_material->SetConstantData(ShaderStage::Pixel, 2, constant);
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
