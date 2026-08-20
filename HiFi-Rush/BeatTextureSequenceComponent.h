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

	struct BeatTextureSequenceDesc
	{
		std::wstring				triggerId{};
		float					beatOffset = 0.f;
		uint32					materialSlot = 0;
		TextureSlot				textureSlot = TextureSlot::BaseColor;
		std::vector<std::wstring>	initialTextureKeys{};
		std::vector<std::wstring>	triggeredTextureKeys{};
		float					framesPerBeat = 1.f;
		float					phaseOffsetBeats = 0.f;
	};

	class BeatTextureSequenceComponent final : public Component
	{
	public:
		BeatTextureSequenceComponent(Resources& resources, const BeatSystem& beatSystem, BeatTextureSequenceDesc desc);

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		bool ResolveMaterial();
		bool ResolveTextures(const std::vector<std::wstring>& textureKeys, std::vector<std::shared_ptr<Texture>>& outTextures) const;
		void ScheduleTriggeredSequence(float startBeat);
		void ResetSequence();
		void UpdateSequenceState();
		void ApplyCurrentFrame();

	private:
		Resources&						_resources;
		const BeatSystem&				_beatSystem;
		BeatTextureSequenceDesc			_desc{};
		TriggerBinding					_triggerBinding{};
		Material*						_material = nullptr;
		std::vector<std::shared_ptr<Texture>>	_initialTextures{};
		std::vector<std::shared_ptr<Texture>>	_triggeredTextures{};
		std::optional<float>				_triggerBeat{};
		std::shared_ptr<Texture>			_currentTexture{};
		bool							_isTriggered = false;
	};
}
