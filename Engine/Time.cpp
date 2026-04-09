#include "Time.h"
#include <wchar.h>

namespace gm
{
	Time::Time()
	{
		QueryPerformanceFrequency(&_cpuFrequency);
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

		TextOut(hDC, 0, 0, szFPS, wcsnlen_s(szFPS, static_cast<size_t>(50)));
	}

	void Time::SetTimeScale(float timeScale)
	{
		GM_ASSERT_RETURN(timeScale >= 0.f, "TimeScale은 0 이상이어야 합니다.");
		_timeScale = timeScale;
	}

	void Time::calcDeltaTime()
	{
		QueryPerformanceCounter(&_curFrequency);

		float diffFrequency = static_cast<float>(_curFrequency.QuadPart - _prevFrequency.QuadPart);
		_unscaledDeltaTime = diffFrequency / static_cast<float>(_cpuFrequency.QuadPart);
		_deltaTime = _unscaledDeltaTime * _timeScale;

		_prevFrequency = _curFrequency;
	}
}
