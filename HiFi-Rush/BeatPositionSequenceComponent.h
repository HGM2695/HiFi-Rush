#pragma once

#include "Component.h"

#include <vector>

namespace gm
{
	class BeatSystem;
	class TransformComponent;

	struct BeatPositionSequenceDesc
	{
		std::vector<Vector3>	positionOffsets{};
		float					beatsPerStep = 1.f;
		float					interpolationSpeed = 6.3f;
	};

	class BeatPositionSequenceComponent : public Component
	{
	public:
		BeatPositionSequenceComponent(const BeatSystem& beatSystem, const BeatPositionSequenceDesc& desc);

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		const BeatSystem&			_beatSystem;
		BeatPositionSequenceDesc	_desc{};
		TransformComponent*			_transform = nullptr;
		Vector3						_initialPosition{};
	};
}
