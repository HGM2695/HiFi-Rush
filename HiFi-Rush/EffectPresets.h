#pragma once

#include "EffectPresetData.h"

#include <string>
#include <vector>
#if GM_ENABLE_DEBUG_TOOLS
#include <optional>
#endif

namespace gm
{
	inline constexpr wchar_t EffectPresetsFilePath[] = L"Data/EffectPresets.bin";

	class EffectPresets
	{
	public:
		bool Load(const std::wstring& filePath);

		const EffectPresetData* Find(const std::wstring& effectId) const;
		const std::vector<EffectPresetData>& GetAll() const { return _presets; }
#if GM_ENABLE_DEBUG_TOOLS
		std::optional<float> FindEmissiveIntensity(const std::wstring& resourceKey) const;
		std::optional<float> FindEmissiveIntensity(const std::wstring& effectId, const std::wstring& resourceKey) const;
		bool SetEmissiveIntensity(const std::wstring& resourceKey, float intensity);
		bool SetEmissiveIntensity(const std::wstring& effectId, const std::wstring& resourceKey, float intensity);
#endif

	private:
		std::vector<EffectPresetData> _presets{};
	};
}
