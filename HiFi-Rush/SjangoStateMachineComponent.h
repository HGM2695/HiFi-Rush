#pragma once

#include "MonsterStateMachineComponent.h"

namespace gm
{
	class SjangoStateMachineComponent final : public MonsterStateMachineComponent
	{
	protected:
		void OnInitialize() override;

	private:
		bool RegisterAnimationClips();
		bool RegisterStates();
	};
}
