#pragma once

namespace gm
{
	namespace LoadingTextureKey
	{
		inline constexpr wchar_t Screen[] = L"T_loading_screen_808";
		inline constexpr wchar_t Note1[] = L"T_loading_screen_note_1";
		inline constexpr wchar_t Note2[] = L"T_loading_screen_note_2";
		inline constexpr wchar_t Note3[] = L"T_loading_screen_note_3";
	}

	bool LoadResources();
}
