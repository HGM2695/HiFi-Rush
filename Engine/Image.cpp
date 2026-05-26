#include "Image.h"
#include "Application.h"
#include "BuiltinGraphicsResources.h"
#include "Material.h"
#include "Renderer.h"
#include "RenderTypes.h"
#include "Resources.h"
#include "Texture.h"

namespace gm
{
	Image::Image()
	{
		SetName(L"Image");
		_samplerDesc.filter = TextureFilter::Point;
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

	void Image::OnRender(const WidgetGeometry& geometry)
	{
		if (_texture == nullptr || geometry.size.x <= 0.f || geometry.size.y <= 0.f)
			return;

		if (_material == nullptr)
			return;

		UIRenderItem item{};
		item.screenCenter = geometry.center;
		item.size = geometry.size;
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
		_material->SetConstantData(ShaderStage::Pixel, 0, constant);
	}
}
