#pragma once

#include <string>

namespace gm
{
	void PlaySound2D(const std::wstring& soundKey, float volume = 1.f);
	void PlayBGM(const std::wstring& soundKey, float volume = 1.f);
	void StopBGM();
}