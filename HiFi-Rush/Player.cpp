#include "Player.h"
#include "../Engine/Transform.h"
#include "../Engine/SpriteRenderer.h"

namespace gm
{
	void Player::OnInitialize()
	{
		AddComponent<Transform>();
		AddComponent<SpriteRenderer>();
	}
}