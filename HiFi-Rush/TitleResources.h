#pragma once

#include "EngineCore.h"
#include <string>

namespace gm::TitleResource
{
	inline constexpr wchar_t FrameTextureDirectory[] = L"UI/Title/";
	inline constexpr uint32 FrameCount = 150;
	inline constexpr float FrameRate = 30.f;
	inline constexpr wchar_t BGMKey[] = L"Title.BGM";
	inline constexpr wchar_t BGMFileName[] = L"Logo.mp3";

	inline std::wstring GetFrameTexturePath(uint32 frameIndex)
	{
		return std::wstring(FrameTextureDirectory) + std::to_wstring(frameIndex) + L".jpg";
	}
}
