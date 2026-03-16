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
		resources.Load<Texture>(L"Player", L"Resources/GameObject/NewPlayer_Left.bmp");

		auto clip = resources.Load<SpriteAnimationClip>(L"Player_Move", L"TestSpriteAnimation");
		clip->SetLoop(false);
	}
}