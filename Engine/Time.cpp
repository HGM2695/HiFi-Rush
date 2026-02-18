#include "Time.h"
#include <wchar.h>

namespace gm
{
	void Time::Initialize()
	{
		// CPU 고유 Hz
		QueryPerformanceFrequency(&_cpuFrequency);

		// 현재의 카운터 값
		QueryPerformanceCounter(&_prevFrequency);
	}

	void Time::Update()
	{
		calcDeltaTime();
	}

	void Time::Render(HDC hDC)
	{
		static float timeElapsed = 0.0f;
		static int frameCount = 0;
		static float fps = 0.0f;
		static wchar_t szFPS[50] = L"";

		timeElapsed += _deltaTime;
		frameCount++;

		if (timeElapsed >= 1.0f)
		{
			fps = static_cast<float>(frameCount) / timeElapsed;
			swprintf_s(szFPS, 50, L"FPS: %.2f", fps);

			frameCount = 0;
			timeElapsed = 0.0f;
		}

		TextOut(hDC, 0, 0, szFPS, wcsnlen_s(szFPS, 50));
	}

	void Time::calcDeltaTime()
	{
		QueryPerformanceCounter(&_curFrequency);

		float diffFrequency = static_cast<float>(_curFrequency.QuadPart - _prevFrequency.QuadPart);
		_deltaTime = diffFrequency / static_cast<float>(_cpuFrequency.QuadPart);

		_prevFrequency = _curFrequency;
	}
}