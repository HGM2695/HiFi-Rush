#include "PlayerMovement.h"
#include "../Engine/Application.h"
#include "../Engine/Input.h"
#include "../Engine/Transform.h"
#include "../Engine/Time.h"
#include "../Engine/GameObject.h"

namespace gm
{
	void PlayerMovement::OnInitialize()
	{
		_transform = GetOwner().GetTransform();
	}

    void PlayerMovement::OnUpdate()
    {
        auto& input = APPLICATION.GetInput();
        float dt = APPLICATION.GetTime().GetDeltaTime();

        math::Vector2 dir = input.GetMoveAxisXY();
        _transform->Translate(dir * _moveSpeed * dt);
    }
}