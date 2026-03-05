#pragma once

#include "../Engine/Application.h"
#include "../Engine/Resources.h"
#include "../Engine/Texture.h"

namespace gm
{
	void LoadResources()
	{
		Resources& resources = APPLICATION.GetResources();
		resources.Load<Texture>(L"OrangeMushroom", L"Resources/GameObject/orange_mushroom.png", ResourceType::Texture);
	}
}



