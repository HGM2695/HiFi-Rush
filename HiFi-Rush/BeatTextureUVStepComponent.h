#pragma once

#include "Component.h"
#include "MathTypes.h"

namespace gm
{
	class BeatSystem;
	class Material;

	struct BeatTextureUVStepDesc
	{
		uint32	materialSlot = 0;
		Vector2	firstOffset{};
		Vector2	secondOffset{};
		float	stepDurationBeats = 1.f;
	};

	class BeatTextureUVStepComponent final : public Component
	{
	public:
		BeatTextureUVStepComponent(const BeatSystem& beatSystem, const BeatTextureUVStepDesc& desc);

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		bool ResolveMaterial();
		void UpdateTextureUVOffset();

	private:
		const BeatSystem&		_beatSystem;
		BeatTextureUVStepDesc	_desc{};
		Material*				_material = nullptr;
	};
}
