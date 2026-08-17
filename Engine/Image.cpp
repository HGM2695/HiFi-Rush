#include "Image.h"
#include "Application.h"
#include "BuiltinGraphicsResources.h"
#include "Material.h"
#include "Renderer.h"
#include "RenderTypes.h"
#include "Resources.h"
#include "Texture.h"

#include <algorithm>

namespace gm
{
	Image::Image()
	{
		SetName(L"Image");
		_samplerDesc.filter = TextureFilter::Linear;
		CreateMaterial();
	}

	Image::Image(const std::wstring& textureName) : Image()
	{
		SetTexture(textureName);
	}

	Image::~Image() = default;

	void Image::SetTexture(const std::shared_ptr<Texture>& texture)
	{
		_texture = texture;
		UpdateMaterial();
	}

	void Image::SetTexture(const std::wstring& textureName)
	{
		_texture = APPLICATION.GetResources().Find<Texture>(textureName);
		GM_ASSERT(_texture, "등록되지 않은 Texture입니다. key = %ls", textureName.c_str());

		UpdateMaterial();
	}

	void Image::SetSamplerDesc(const SamplerDesc& desc)
	{
		_samplerDesc = desc;
		UpdateMaterial();
	}

	void Image::SetOpacity(float opacity)
	{
		_opacity = std::clamp(opacity, 0.f, 1.f);
		UpdateMaterial();
	}

	void Image::SetColorBlend(Color color, float ratio)
	{
		_blendColor = color;
		_blendRatio = std::clamp(ratio, 0.f, 1.f);
		UpdateMaterial();
	}

	void Image::SetColorBlendRatio(float ratio)
	{
		_blendRatio = std::clamp(ratio, 0.f, 1.f);
		UpdateMaterial();
	}

	void Image::SetFillRatio(float ratio)
	{
		_fillRatio = std::clamp(ratio, 0.f, 1.f);
		UpdateMaterial();
	}

	void Image::SetFillMode(ImageFillMode fillMode)
	{
		GM_ASSERT_RETURN(fillMode < ImageFillMode::Count, "지원하지 않는 Image Fill Mode입니다.");
		_fillMode = fillMode;
		UpdateMaterial();
	}

	void Image::SetRadialFill(float startAngle, float sweepAngle)
	{
		GM_ASSERT_RETURN(sweepAngle > 0.f, "Radial Fill의 Sweep Angle은 0보다 커야 합니다.");
		_fillMode = ImageFillMode::Radial;
		_radialStartAngle = startAngle;
		_radialSweepAngle = sweepAngle;
		UpdateMaterial();
	}

	void Image::OnRender(const WidgetGeometry& geometry)
	{
		if (_texture == nullptr || geometry.size.x <= 0.f || geometry.size.y <= 0.f)
			return;

		if (_material == nullptr)
			return;

		UIRenderItem item{};
		item.screenCenter = geometry.center;
		item.size = geometry.size;
		item.rotation = geometry.rotation;
		item.material = _material.get();

		APPLICATION.GetRenderer().SubmitUI(item);
	}

	void Image::CreateMaterial()
	{
		if (_material)
			return;

		_material = std::make_unique<Material>(
			Material::MaterialBuilder(APPLICATION.GetResources())
				.SetCullMode(CullMode::None)
				.SetDepthEnable(false)
				.SetDepthWriteEnable(false)
				.SetBlendEnable(true)
				.SetVertexShader(BuiltinResourceKey::QuadVS)
				.SetPixelShader(BuiltinResourceKey::SpriteTexturePS)
				.SetSampler(TextureSlot::BaseColor, _samplerDesc)
				.Build()
		);

		UpdateMaterial();
	}

	void Image::UpdateMaterial()
	{
		if (_material == nullptr)
			return;

		_material->SetTexture(TextureSlot::BaseColor, _texture);
		_material->SetSamplerDesc(TextureSlot::BaseColor, _samplerDesc);

		SpriteConstantPS constant{};
		constant.blendColor = _blendColor;
		constant.blendRatio = _blendRatio;
		constant.opacity = _opacity;
		constant.fillRatio = _fillRatio;
		constant.fillMode = static_cast<uint32>(_fillMode);
		constant.radialStartAngle = _radialStartAngle;
		constant.radialSweepAngle = _radialSweepAngle;
		_material->SetConstantData(ShaderStage::Pixel, 0, constant);
	}
}
