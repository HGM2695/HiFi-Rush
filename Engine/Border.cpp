#include "Border.h"
#include "Application.h"
#include "BuiltinGraphicsResources.h"
#include "Material.h"
#include "Renderer.h"
#include "RenderTypes.h"
#include "Resources.h"
#include "Rect.h"
#include <algorithm>

namespace gm
{
	namespace
	{
		std::unique_ptr<Material> CreateSolidColorUIMaterial()
		{
			return std::make_unique<Material>(
				Material::MaterialBuilder(APPLICATION.GetResources())
					.SetCullMode(CullMode::None)
					.SetDepthEnable(false)
					.SetDepthWriteEnable(false)
					.SetBlendEnable(true)
					.SetVertexShader(BuiltinResourceKey::QuadVS)
					.SetPixelShader(BuiltinResourceKey::SolidColorPS)
					.Build()
			);
		}

		void SubmitUIQuad(const Vector2& center, const Vector2& size, const Material* material)
		{
			UIRenderItem item{};
			item.screenCenter = center;
			item.size = size;
			item.material = material;

			APPLICATION.GetRenderer().SubmitUI(item);
		}
	}

	Border::Border()
	{
		SetName(L"Border");
		CreateBackgroundMaterial();
		CreateOutlineMaterial();
	}

	Border::~Border() = default;

	void Border::OnRender(const WidgetGeometry& geometry)
	{
		if (geometry.size.x <= 0.f || geometry.size.y <= 0.f)
			return;

		SubmitBackGround(geometry);
		SubmitOutline(geometry);
	}

	void Border::SubmitBackGround(const WidgetGeometry& geometry)
	{
		if (_backgroundMaterial == nullptr)
			return;

		SubmitUIQuad(geometry.center, geometry.size, _backgroundMaterial.get());
	}

	void Border::SubmitOutline(const WidgetGeometry& geometry)
	{
		if (_borderThickness <= 0.f)
			return;

		if (_outlineMaterial == nullptr)
			return;

		Rect rect = Rect::FromCenterSize(geometry.center, geometry.size);
		const float horizontalLength = std::max(0.f, geometry.size.x - _borderThickness * 2.f);

		SubmitUIQuad(Vector2{ rect.left + 0.5f * _borderThickness, geometry.center.y }, Vector2{ _borderThickness, geometry.size.y }, _outlineMaterial.get());
		SubmitUIQuad(Vector2{ rect.Right() - 0.5f * _borderThickness, geometry.center.y }, Vector2{ _borderThickness, geometry.size.y }, _outlineMaterial.get());
		SubmitUIQuad(Vector2{ geometry.center.x, rect.top + 0.5f * _borderThickness }, Vector2{ horizontalLength, _borderThickness }, _outlineMaterial.get());
		SubmitUIQuad(Vector2{ geometry.center.x, rect.Bottom() - 0.5f * _borderThickness }, Vector2{ horizontalLength, _borderThickness }, _outlineMaterial.get());
	}

	void Border::SetBackgroundColor(Color color)
	{
		_backgroundColor = color;
		UpdateBackgroundMaterial();
	}

	void Border::SetOutlineColor(Color color)
	{
		_outlineColor = color;
		UpdateOutlineMaterial();
	}

	void Border::CreateBackgroundMaterial()
	{
		if (_backgroundMaterial)
			return;

		_backgroundMaterial = CreateSolidColorUIMaterial();
		UpdateBackgroundMaterial();
	}

	void Border::CreateOutlineMaterial()
	{
		if (_outlineMaterial)
			return;

		_outlineMaterial = CreateSolidColorUIMaterial();
		UpdateOutlineMaterial();
	}

	void Border::UpdateBackgroundMaterial()
	{
		if (_backgroundMaterial == nullptr)
			return;

		ColorConstantPS constant{};
		constant.color = _backgroundColor;
		_backgroundMaterial->SetConstantData(ShaderStage::Pixel, 0, constant);
	}

	void Border::UpdateOutlineMaterial()
	{
		if (_outlineMaterial == nullptr)
			return;

		ColorConstantPS constant{};
		constant.color = _outlineColor;
		_outlineMaterial->SetConstantData(ShaderStage::Pixel, 0, constant);
	}
}
