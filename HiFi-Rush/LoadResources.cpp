#include "LoadResources.h"
#include "Application.h"
#include "IGraphicsResourceFactory.h"
#include "PathUtil.h"
#include "Paths.h"
#include "Resources.h"
#include "Renderer.h"
#include "SoundWave.h"
#include "Texture.h"
#include "TitleResources.h"

#include <array>

namespace gm
{
	namespace
	{
		bool LoadTexture(Resources& resources, IGraphicsResourceFactory& resourceFactory, const std::wstring& relativePath, TextureColorSpace colorSpace = TextureColorSpace::SRGB)
		{
			const std::wstring key = GetFileNameWithoutExtension(relativePath);
			if (resources.Find<Texture>(key))
				return true;

			TextureLoadDesc desc{};
			desc.path = GetTexturePath(relativePath);
			desc.colorSpace = colorSpace;

			std::shared_ptr<Texture> texture = resourceFactory.LoadTexture(desc);
			GM_ASSERT_RETURN_VAL(texture, false, "Texture 생성에 실패했습니다. key=%ls", key.c_str());
			GM_ASSERT_RETURN_VAL(resources.Add(key, texture), false, "Texture 등록에 실패했습니다. key=%ls", key.c_str());
			return true;
		}

		bool LoadSoundWave(Resources& resources, const std::wstring& key, const std::wstring& relativePath)
		{
			if (resources.Find<SoundWave>(key))
				return true;

			SoundWaveDesc desc{};
			desc.path = GetAudioPath(relativePath);

			std::shared_ptr<SoundWave> sound = SoundWave::Create(desc);
			GM_ASSERT_RETURN_VAL(sound, false, "SoundWave 생성에 실패했습니다. key=%ls", key.c_str());
			GM_ASSERT_RETURN_VAL(resources.Add(key, sound), false, "SoundWave 등록에 실패했습니다. key=%ls", key.c_str());
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

		constexpr wchar_t SpotLightCookieTexturePath[] = L"Mesh/Light/T_SpotCookie_Star.png";
		if (LoadTexture(resources, resourceFactory, SpotLightCookieTexturePath, TextureColorSpace::Linear) == false)
			return false;
		APPLICATION.GetRenderer().SetSpotLightCookieTexture(resources.Find<Texture>(GetFileNameWithoutExtension(SpotLightCookieTexturePath)));

		if (LoadSoundWave(resources, TitleResource::BGMKey, TitleResource::BGMFileName) == false)
			return false;

		return true;
	}
}
