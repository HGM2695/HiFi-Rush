#pragma once

#include "Component.h"
#include "GraphicsTypes.h"
#include "TriggerBinding.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace gm
{
	class BeatSystem;
	class Material;
	class Resources;
	class Texture;

	struct MaterialTextureOverrideDesc
	{
		uint32			materialSlot = 0;
		TextureSlot		textureSlot = TextureSlot::BaseColor;
		std::wstring	textureKey{};
	};

	struct TriggeredMaterialOverrideDesc
	{
		std::wstring						triggerId{};
		float							beatOffset = 0.f;
		std::vector<MaterialTextureOverrideDesc>	overrides{};
	};

	class TriggeredMaterialOverrideComponent final : public Component
	{
	public:
		TriggeredMaterialOverrideComponent(Resources& resources, const BeatSystem& beatSystem, TriggeredMaterialOverrideDesc desc);

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		struct MaterialTextureOverride
		{
			Material*				material = nullptr;
			TextureSlot			textureSlot = TextureSlot::BaseColor;
			std::shared_ptr<Texture>	initialTexture{};
			std::shared_ptr<Texture>	overrideTexture{};
		};

		bool ResolveOverrides();
		Material* ResolveMaterial(uint32 materialSlot);
		void ScheduleOverride(float startBeat);
		void ResetOverrides();
		void ApplyOverrides();

	private:
		Resources&						_resources;
		const BeatSystem&				_beatSystem;
		TriggeredMaterialOverrideDesc	_desc{};
		TriggerBinding					_triggerBinding{};
		std::vector<MaterialTextureOverride>	_overrides{};
		std::optional<float>				_triggerBeat{};
		bool							_isApplied = false;
	};
}
