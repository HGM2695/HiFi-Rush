#pragma once

#include "Component.h"
#include "MathTypes.h"

namespace gm
{
	class BeatSystem;
	class Material;

	struct BeatTextureUVScrollDesc
	{
		uint32	materialSlot = 0;
		Vector2	offsetPerBeat{};
	};

	class BeatTextureUVScrollComponent final : public Component
	{
	public:
		BeatTextureUVScrollComponent(const BeatSystem& beatSystem, const BeatTextureUVScrollDesc& desc);

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		bool ResolveMaterial();
		void UpdateTextureUVOffset();

	private:
		const BeatSystem&		_beatSystem;
		BeatTextureUVScrollDesc	_desc{};
		Material*				_material = nullptr;
	};
}
