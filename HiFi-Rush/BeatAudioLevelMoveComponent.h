#pragma once

#include "Component.h"

namespace gm
{
	class BeatSystem;
	class TransformComponent;

	struct BeatAudioLevelMoveDesc
	{
		Vector3	direction{ 0.f, 1.f, 0.f };
		float	maxDistance = 1.f;
		float	cycleBeats = 1.f;
		float	phaseOffsetBeats = 0.f;
	};

	class BeatAudioLevelMoveComponent : public Component
	{
	public:
		BeatAudioLevelMoveComponent(const BeatSystem& beatSystem, const BeatAudioLevelMoveDesc& desc);

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		const BeatSystem&		_beatSystem;
		BeatAudioLevelMoveDesc	_desc{};
		TransformComponent*		_transform = nullptr;
		Vector3					_initialPosition{};
		Vector3					_direction{};
	};
}
