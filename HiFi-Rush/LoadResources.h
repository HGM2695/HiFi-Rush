#pragma once

#include "../Engine/Application.h"
#include "../Engine/Resources.h"
#include "../Engine/Texture.h"
#include "../Engine/SpriteAnimationClip.h"

namespace gm
{
	void LoadResources()
	{
		Resources& resources = APPLICATION.GetResources();
		resources.Load<Texture>(L"OrangeMushroom", L"Resources/GameObject/orange_mushroom.png");
		resources.Load<Texture>(L"Xanadu", L"Resources/GameObject/Xanadu.png");
		resources.Load<Texture>(L"PlayerLeft", L"Resources/GameObject/NewPlayer_Left.bmp");
		resources.Load<Texture>(L"PlayerRight", L"Resources/GameObject/NewPlayer_Right.bmp");

		auto idleLeftClip = resources.Load<SpriteAnimationClip>(L"Player_IdleLeft", L"TestSpriteIdleLeft");
		auto moveLeftClip = resources.Load<SpriteAnimationClip>(L"Player_MoveLeft", L"TestSpriteMoveLeft");
		auto idleRightClip = resources.Load<SpriteAnimationClip>(L"Player_IdleRight", L"TestSpriteIdleRight");
		auto moveRightClip = resources.Load<SpriteAnimationClip>(L"Player_MoveRight", L"TestSpriteMoveRight");
	}
}
