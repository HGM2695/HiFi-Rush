#include "LoadResources.h"
#include "Application.h"
#include "IGraphicsResourceFactory.h"
#include "PathUtil.h"
#include "Paths.h"
#include "Resources.h"
#include "Texture.h"

#include <array>

namespace gm
{
	namespace
	{
		bool LoadTexture(Resources& resources, IGraphicsResourceFactory& resourceFactory, const std::wstring& relativePath)
		{
			const std::wstring key = GetFileNameWithoutExtension(relativePath);
			if (resources.Find<Texture>(key))
				return true;

			TextureDesc desc{};
			desc.path = GetTexturePath(relativePath);

			std::shared_ptr<Texture> texture = resourceFactory.CreateTexture(desc);
			GM_ASSERT_RETURN_VAL(texture, false, "UI Texture 생성에 실패했습니다. key=%ls", key.c_str());
			GM_ASSERT_RETURN_VAL(resources.Add(key, texture), false, "UI Texture 등록에 실패했습니다. key=%ls", key.c_str());
			return true;
		}
	}

	bool LoadResources()
	{
		constexpr std::array<const wchar_t*, 4> loadingScreenTextures =
		{{
			L"UI/Loading/T_loading_screen_808.dds",
			L"UI/Loading/T_loading_screen_note_1.dds",
			L"UI/Loading/T_loading_screen_note_2.dds",
			L"UI/Loading/T_loading_screen_note_3.dds",
		}};

		Resources& resources = APPLICATION.GetResources();
		IGraphicsResourceFactory& resourceFactory = APPLICATION.GetGraphicsResourceFactory();
		for (const wchar_t* relativePath : loadingScreenTextures)
		{
			if (LoadTexture(resources, resourceFactory, relativePath) == false)
				return false;
		}

		return true;
	}
}
