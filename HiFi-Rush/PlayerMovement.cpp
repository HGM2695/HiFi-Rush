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
		_ownerTransform = GetOwner().GetTransform();
	}

    void PlayerMovement::OnUpdate()
    {
        auto& input = APPLICATION.GetInput();
        float dt = APPLICATION.GetTime().GetDeltaTime();

        math::Vector2 dir = input.GetMoveAxisXY();
        _isMoving = (dir._x != 0.f || dir._y != 0.f);

        if (dir._x < 0.f)
            _ownerTransform->SetForward({ -1.f, 0.f });
        else if (dir._x > 0.f)
            _ownerTransform->SetForward({ 1.f, 0.f });

        _ownerTransform->Translate(dir * _moveSpeed * dt);
    }
}
