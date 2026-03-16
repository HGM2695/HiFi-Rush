#pragma once

#include "../Engine/Application.h"
#include "../Engine/Resources.h"
#include "../Engine/Texture.h"
#include "../Engine/SpriteAnimationClip.h"

namespace gm
{
	void TempAnimationClipRoad();

	void LoadResources()
	{
		Resources& resources = APPLICATION.GetResources();
		resources.Load<Texture>(L"OrangeMushroom", L"Resources/GameObject/orange_mushroom.png");
		resources.Load<Texture>(L"Xanadu", L"Resources/GameObject/Xanadu.png");
		resources.Load<Texture>(L"PlayerLeft", L"Resources/GameObject/NewPlayer_Left.bmp");
		resources.Load<Texture>(L"PlayerRight", L"Resources/GameObject/NewPlayer_Right.bmp");

		TempAnimationClipRoad();
	}

	void TempAnimationClipRoad()
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
	}
}