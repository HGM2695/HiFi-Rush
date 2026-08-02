#include "LoadResources.h"
#include "Application.h"
#include "IGraphicsResourceFactory.h"
#include "Paths.h"
#include "Resources.h"
#include "Texture.h"

#include <array>

namespace gm
{
	namespace
	{
		struct TextureLoadEntry
		{
			const wchar_t* key = nullptr;
			const wchar_t* fileName = nullptr;
		};

		bool LoadTexture(Resources& resources, IGraphicsResourceFactory& resourceFactory, const TextureLoadEntry& entry)
		{
			if (resources.Find<Texture>(entry.key))
				return true;

			TextureDesc desc{};
			desc.path = GetTexturePath(std::wstring(L"UI/Loading/") + entry.fileName);

			std::shared_ptr<Texture> texture = resourceFactory.CreateTexture(desc);
			GM_ASSERT_RETURN_VAL(texture, false, "로딩 화면 Texture 생성에 실패했습니다. key=%ls", entry.key);
			GM_ASSERT_RETURN_VAL(resources.Add(entry.key, texture), false, "로딩 화면 Texture 등록에 실패했습니다. key=%ls", entry.key);
			return true;
		}
	}

	bool LoadResources()
	{
		constexpr std::array<TextureLoadEntry, 4> loadingTextures =
		{{
			{ LoadingTextureKey::Screen, L"T_loading_screen_808.dds" },
			{ LoadingTextureKey::Note1, L"T_loading_screen_note_1.dds" },
			{ LoadingTextureKey::Note2, L"T_loading_screen_note_2.dds" },
			{ LoadingTextureKey::Note3, L"T_loading_screen_note_3.dds" },
		}};

		Resources& resources = APPLICATION.GetResources();
		IGraphicsResourceFactory& resourceFactory = APPLICATION.GetGraphicsResourceFactory();
		for (const TextureLoadEntry& entry : loadingTextures)
		{
			if (LoadTexture(resources, resourceFactory, entry) == false)
				return false;
		}

		return true;
	}
}
