#include "Player.h"
#include "../Engine/Transform.h"
#include "../Engine/SpriteRenderer.h"

namespace gm
{
	void Player::OnInitialize()
	{
		AddComponent<Transform>();
		SpriteRenderer* spriteRenderer = AddComponent<SpriteRenderer>();

		// 임시 이미지
		spriteRenderer->ImageLoad(L"Resources/GameObject/orange_mushroom.png");
	}
}