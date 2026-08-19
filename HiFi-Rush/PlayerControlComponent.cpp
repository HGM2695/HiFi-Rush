#include "PlayerControlComponent.h"

#include "CameraFollowComponent.h"
#include "ChiMoveComponent.h"
#include "ChiStateMachineComponent.h"

namespace gm
{
	PlayerControlComponent::PlayerControlComponent(ChiMoveComponent& moveComponent, ChiStateMachineComponent& stateMachineComponent, CameraFollowComponent& cameraFollowComponent)
		: _moveComponent(moveComponent), _stateMachineComponent(stateMachineComponent), _cameraFollowComponent(cameraFollowComponent)
	{}

	void PlayerControlComponent::BlockControls(const void* source, PlayerControlMask controls)
	{
		GM_ASSERT_RETURN(source, "Player Control 차단 요청자는 nullptr일 수 없습니다.");

		if (controls == PlayerControl::None)
		{
			ReleaseControls(source);
			return;
		}

		_controlBlocks[source] = controls;
		ApplyControl();
	}

	void PlayerControlComponent::ReleaseControls(const void* source)
	{
		if (source == nullptr || _controlBlocks.erase(source) == 0)
			return;

		ApplyControl();
	}

	bool PlayerControlComponent::IsControlEnabled(PlayerControlMask control) const
	{
		return (_controlMask & control) == 0;
	}

	void PlayerControlComponent::ApplyControl()
	{
		_controlMask = PlayerControl::None;
		for (const auto& [source, controls] : _controlBlocks)
			_controlMask |= controls;

		_moveComponent.SetMovementEnabled(IsControlEnabled(PlayerControl::Movement));
		_stateMachineComponent.SetInputEnabled(IsControlEnabled(PlayerControl::Action));
		_cameraFollowComponent.SetMouseControlEnabled(IsControlEnabled(PlayerControl::Camera));
	}
}
