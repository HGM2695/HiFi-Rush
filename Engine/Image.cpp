#include "Image.h"
#include "Application.h"
#include "BuiltinGraphicsResources.h"
#include "Renderer.h"
#include "RenderTypes.h"
#include "Resources.h"
#include "Sampler.h"
#include "Texture.h"

namespace gm
{
	Image::Image()
	{
		SetName(L"Image");
		SetSampler(BuiltinResourceKey::PointSampler);
	}

	Image::Image(const std::wstring& textureName) : Image()
	{
		SetTexture(textureName);
	}

	Image::~Image() = default;

	void Image::SetTexture(const std::wstring& textureName)
	{
		_texture = APPLICATION.GetResources().Find<Texture>(textureName);
		GM_ASSERT(_texture, "등록되지 않은 텍스처 [%ls] 입니다.", textureName.c_str());
	}

	void Image::SetSampler(const std::wstring& samplerName)
	{
		_sampler = APPLICATION.GetResources().Find<Sampler>(samplerName);
		GM_ASSERT(_sampler, "등록되지 않은 샘플러 [%ls] 입니다.", samplerName.c_str());
	}

	void Image::OnRender(const WidgetGeometry& geometry)
	{
		if (_texture == nullptr || geometry.size.x <= 0.f || geometry.size.y <= 0.f)
			return;

		TextureQuadRenderItem item{};
		item.screenCenter = geometry.center;
		item.size = geometry.size;
		item.texture = _texture;
		item.sampler = _sampler;

		APPLICATION.GetRenderer().SubmitTextureQuad(item);
	}
}
