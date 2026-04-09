#pragma once

#include "Application.h"
#include "Resources.h"
#include "Texture.h"
#include "SpriteAnimationClip.h"
#include "SoundWave.h"
#include "Paths.h"

namespace gm
{
	void LoadTexture(Resources& resources);
	void LoadTempAnimationClip(Resources& resources);
	void LoadAudio(Resources& resources);

	void LoadResources()
	{
		Resources& resources = APPLICATION.GetResources();

		LoadTexture(resources);
		LoadTempAnimationClip(resources);
		LoadAudio(resources);
	}

	void LoadTexture(Resources& resources)
	{
		resources.Load<Texture>(L"OrangeMushroom", GetTexturePath(L"Test/orange_mushroom.png"));
		resources.Load<Texture>(L"Xanadu", GetTexturePath(L"Test/Xanadu.png"));
		resources.Load<Texture>(L"PlayerLeft", GetTexturePath(L"Test/NewPlayer_Left.bmp"));
		resources.Load<Texture>(L"PlayerRight", GetTexturePath(L"Test/NewPlayer_Right.bmp"));
	}

	void LoadTempAnimationClip(Resources& resources)
	{
		// 추후 Animation Data 파일로 부터 파싱하는 구조로 변경 예정
		// IdleLeft
		std::shared_ptr<SpriteAnimationClip> spriteAnimatinoClip = std::make_shared< SpriteAnimationClip>();
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
		spriteAnimatinoClip = std::make_shared<SpriteAnimationClip>();
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
		spriteAnimatinoClip = std::make_shared< SpriteAnimationClip>();
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
		spriteAnimatinoClip = std::make_shared< SpriteAnimationClip>();
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
		resources.Load<SoundWave>(L"TestBGM", GetAudioPath(L"Lake Of Oblivion.mp3"));
		resources.Load<SoundWave>(L"Two", GetAudioPath(L"Two.wav"));
	}
}
