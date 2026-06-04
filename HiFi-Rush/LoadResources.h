#pragma once

#include "Application.h"
#include "Resources.h"
#include "Texture.h"
#include "SpriteAnimationClip.h"
#include "SoundWave.h"
#include "Paths.h"
#include "PathUtil.h"
#include "VertexTypes.h"
#include "IGraphicsResourceFactory.h"
#include "BinaryModelLoader.h"
#include "StaticMesh.h"
#include "SkeletalMesh.h"
#include "SkeletalAnimationClip.h"
#include "StringUtil.h"
#include "ChiAnimationTypes.h"
#include <filesystem>
#include <vector>

namespace gm
{
	void LoadTexture(Resources& resources);
	void LoadMeshTexture(Resources& resources);
	void LoadStaticMesh(Resources& resources);
	void LoadSkeletalMesh(Resources& resources);
	void LoadTempAnimationClip(Resources& resources);
	void LoadAudio(Resources& resources);

	void LoadResources()
	{
		Resources& resources = APPLICATION.GetResources();

		LoadTexture(resources);
		LoadMeshTexture(resources);
		LoadStaticMesh(resources);
		LoadSkeletalMesh(resources);
		//LoadTempAnimationClip(resources);
		LoadAudio(resources);
	}

	void LoadTexture(Resources& resources)
	{
		IGraphicsResourceFactory& factory = APPLICATION.GetGraphicsResourceFactory();

		auto LoadTextureResource = [&](const std::wstring& key, const std::wstring& path)
		{
			TextureDesc desc{};
			desc.path = path;

			std::shared_ptr<Texture> texture = factory.CreateTexture(desc);
			resources.Add(key, texture);
		};

		LoadTextureResource(L"OrangeMushroom", GetTexturePath(L"Test/orange_mushroom.png"));
		LoadTextureResource(L"Xanadu", GetTexturePath(L"Test/Xanadu.png"));
		LoadTextureResource(L"PlayerLeft", GetTexturePath(L"Test/NewPlayer_Left.bmp"));
		LoadTextureResource(L"PlayerRight", GetTexturePath(L"Test/NewPlayer_Right.bmp"));
	}

	void LoadMeshTexture(Resources& resources)
	{
		IGraphicsResourceFactory& factory = APPLICATION.GetGraphicsResourceFactory();
		const std::filesystem::path meshTexturePath = GetTexturePath(L"Mesh");

		for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(meshTexturePath))
		{
			if (entry.is_regular_file() == false)
				continue;

			const std::wstring key = GetFileNameWithoutExtension(entry.path().wstring());
			if (key.empty())
				continue;

			TextureDesc desc{};
			desc.path = entry.path().wstring();

			std::shared_ptr<Texture> texture = factory.CreateTexture(desc);
			resources.Add(key, texture);
		}
	}

	void LoadStaticMesh(Resources& resources)
	{
		BinaryModelLoader loader;
		ModelData environment97Data = loader.Load(GetModelPath(L"Binary/Environment/Environment97.bin"));

		std::shared_ptr<StaticMesh> environment97 = StaticMesh::Create(environment97Data, APPLICATION.GetGraphicsResourceFactory());
		GM_ASSERT_RETURN(environment97, "Environment97 StaticMesh 생성에 실패했습니다.");

		resources.Add(L"Environment97", environment97);
	}

	void LoadSkeletalMesh(Resources& resources)
	{
		BinaryModelLoader loader;
		ModelData chiModelData = loader.Load(GetModelPath(L"Binary/Characters/Chi.bin"));

		 std::shared_ptr<SkeletalMesh> chi = SkeletalMesh::Create(chiModelData, APPLICATION.GetGraphicsResourceFactory());
		 GM_ASSERT_RETURN(chi, "chi SkeletalMesh 생성에 실패했습니다.");

		 resources.Add(L"chi", chi);

		for (uint32 animationIndex = 0; animationIndex < chiModelData.animations.size(); ++animationIndex)
		{
			const SkeletalAnimationClipData& clipData = chiModelData.animations[animationIndex];

			std::shared_ptr<SkeletalAnimationClip> clip = SkeletalAnimationClip::Create(clipData);
			GM_ASSERT_RETURN(clip, "chi SkeletalAnimationClip 생성에 실패했습니다.");

			const std::wstring animationKey = GetChiAnimationKey(static_cast<ChiAnimationId>(animationIndex));
			resources.Add(animationKey, clip);

			if (animationIndex == 27)
				resources.Add(L"chi.DefaultAnimation", clip);

			GM_LOG("Skeletal Animation Clip Load. Key: %s", WideToUtf8(animationKey).c_str());
		}
	}

	void LoadTempAnimationClip(Resources& resources)
	{
		// 추후 Animation Data 파일로 부터 파싱하는 구조로 변경 예정
		// IdleLeft

		std::shared_ptr<SpriteAnimationClip> spriteAnimatinoClip = SpriteAnimationClip::Create(SpriteAnimationClipDesc());
		auto texture = APPLICATION.GetResources().Find<Texture>(L"PlayerLeft");
		GM_ASSERT(texture, "SpriteAnimationClip이 사용하는 Texture가 존재하지 않습니다.");
		spriteAnimatinoClip->SetTexture(texture);

		int frameWidth = static_cast<int>(texture->GetWidth() / 4);
		int frameHeight = static_cast<int>(texture->GetHeight() / 9);

		for (int i = 0; i < 3; ++i)
			spriteAnimatinoClip->AddFrame({ frameWidth * i, 0, frameWidth, frameHeight, 0.5f });
		spriteAnimatinoClip->AddFrame({ frameWidth, 0, frameWidth, frameHeight, 0.5f });
		APPLICATION.GetResources().Add(L"Player_IdleLeft", spriteAnimatinoClip);

		// MoveLeft
		spriteAnimatinoClip = SpriteAnimationClip::Create(SpriteAnimationClipDesc());
		texture = APPLICATION.GetResources().Find<Texture>(L"PlayerLeft");
		GM_ASSERT(texture, "SpriteAnimationClip이 사용하는 Texture가 존재하지 않습니다.");
		spriteAnimatinoClip->SetTexture(texture);

		frameWidth = static_cast<int>(texture->GetWidth() / 4);
		frameHeight = static_cast<int>(texture->GetHeight() / 9);

		for (int i = 0; i < 3; ++i)
			spriteAnimatinoClip->AddFrame({ frameWidth * i, frameHeight, frameWidth, frameHeight, 0.3f });
		spriteAnimatinoClip->AddFrame({ frameWidth, frameHeight, frameWidth, frameHeight, 0.3f });
		APPLICATION.GetResources().Add(L"Player_MoveLeft", spriteAnimatinoClip);

		spriteAnimatinoClip->AddNotify(0.3f, L"MoveLeftStep");
		spriteAnimatinoClip->AddNotify(0.9f, L"MoveLeftStep");

		// IdleRight
		spriteAnimatinoClip = SpriteAnimationClip::Create(SpriteAnimationClipDesc());
		texture = APPLICATION.GetResources().Find<Texture>(L"PlayerRight");
		GM_ASSERT(texture, "SpriteAnimationClip이 사용하는 Texture가 존재하지 않습니다.");
		spriteAnimatinoClip->SetTexture(texture);

		frameWidth = static_cast<int>(texture->GetWidth() / 4);
		frameHeight = static_cast<int>(texture->GetHeight() / 9);

		for (int i = 0; i < 3; ++i)
			spriteAnimatinoClip->AddFrame({ frameWidth * i, 0, frameWidth, frameHeight, 0.5f });
		spriteAnimatinoClip->AddFrame({ frameWidth, 0, frameWidth, frameHeight, 0.5f });
		APPLICATION.GetResources().Add(L"Player_IdleRight", spriteAnimatinoClip);

		// MoveRight
		spriteAnimatinoClip = SpriteAnimationClip::Create(SpriteAnimationClipDesc());
		texture = APPLICATION.GetResources().Find<Texture>(L"PlayerRight");
		GM_ASSERT(texture, "SpriteAnimationClip이 사용하는 Texture가 존재하지 않습니다.");
		spriteAnimatinoClip->SetTexture(texture);

		frameWidth = static_cast<int>(texture->GetWidth() / 4);
		frameHeight = static_cast<int>(texture->GetHeight() / 9);

		for (int i = 0; i < 3; ++i)
			spriteAnimatinoClip->AddFrame({ frameWidth * i, frameHeight, frameWidth, frameHeight, 0.3f });
		spriteAnimatinoClip->AddFrame({ frameWidth, frameHeight, frameWidth, frameHeight, 0.3f });
		APPLICATION.GetResources().Add(L"Player_MoveRight", spriteAnimatinoClip);

		spriteAnimatinoClip->AddNotify(0.3f, L"MoveRightStep");
		spriteAnimatinoClip->AddNotify(0.9f, L"MoveRightStep");
	}

	void LoadAudio(Resources& resources)
	{
		SoundWaveDesc desc{};

		desc.path = GetAudioPath(L"Lake Of Oblivion.mp3");
		resources.Load<SoundWave>(L"TestBGM", desc);

		desc.path = GetAudioPath(L"Two.wav");
		resources.Load<SoundWave>(L"Two", desc);
	}
}
