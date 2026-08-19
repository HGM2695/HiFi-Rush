#pragma once

#include "Component.h"
#include "PlayerControlTypes.h"

#include <unordered_map>

namespace gm
{
	class CameraFollowComponent;
	class ChiMoveComponent;
	class ChiStateMachineComponent;

	class PlayerControlComponent final : public Component
	{
	public:
		PlayerControlComponent(ChiMoveComponent& moveComponent, ChiStateMachineComponent& stateMachineComponent, CameraFollowComponent& cameraFollowComponent);

		void BlockControls(const void* source, PlayerControlMask controls);
		void ReleaseControls(const void* source);
		bool IsControlEnabled(PlayerControlMask control) const;

	private:
		void ApplyControl();

	private:
		std::unordered_map<const void*, PlayerControlMask>	_controlBlocks{};
		ChiMoveComponent&									_moveComponent;
		ChiStateMachineComponent&							_stateMachineComponent;
		CameraFollowComponent&								_cameraFollowComponent;
		PlayerControlMask									_controlMask = PlayerControl::None;
	};
}
