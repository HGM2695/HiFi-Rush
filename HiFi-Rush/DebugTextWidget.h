#pragma once

#include "MaterialTypes.h"
#include "UserWidget.h"
#include "WeakGameObjectPtr.h"

#include <optional>
#include <vector>

#if GM_ENABLE_DEBUG_TOOLS

namespace gm
{
	class BeatSystem;

	class DebugTextWidget : public UserWidget
	{
		enum class Page
		{
			General,
			Rendering,
			Shadow,
			Atmosphere,
			PostProcess,
			Material,
			Audio
		};

	public:
		explicit DebugTextWidget(const BeatSystem& beatSystem) : _beatSystem(beatSystem) {}
		~DebugTextWidget() override;

	protected:
		std::unique_ptr<Widget> BuildWidgetTree() override;

		virtual void			OnTick(float deltaTime) override;

	private:
		void RestoreMaterialHighlight();
		void ApplyMaterialHighlight();
		void InitializeAudioDebug();
		void ChangeAudioBGM(int32 direction);
		void PlaySelectedAudioBGM();
		void AdjustAudioSongOffset(float amount);
		void RecordAudioInput(bool strongAttack);
		void ResetAudioInputSamples();

	private:
		const BeatSystem&	_beatSystem;
		uint32				_callCount = 0;
		float				_fps = 0.f;
		float				_accTime = 0.f;
		float				_beatPulseTime = 0.f;
		Page				_page = Page::General;
		uint32				_effectSettingIndex = 0;
		uint32				_shadowSettingIndex = 0;
		uint32				_atmosphereSettingIndex = 0;
		uint32				_postProcessSettingIndex = 0;
		std::optional<uint32>	_selectedEnvironmentModelIndex{};
		WeakGameObjectPtr		_selectedMaterialObject{};
		uint32				_materialSlotIndex = 0;
		MaterialColorData		_materialOriginalColorData{};
		float				_materialHighlightTime = 0.f;
		bool				_hasMaterialHighlight = false;
		std::vector<float>	_audioSongOffsets{};
		uint32				_audioBGMIndex = 0;
		uint32				_audioInputSampleCount = 0;
		float				_audioInputErrorSum = 0.f;
	};
}

#endif
