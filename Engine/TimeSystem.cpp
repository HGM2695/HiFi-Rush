#include "TimeSystem.h"
#include <wchar.h>

namespace gm
{
	TimeSystem::TimeSystem()
	{
		QueryPerformanceFrequency(&_cpuFrequency);
		QueryPerformanceCounter(&_prevFrequency);
	}

	void TimeSystem::Tick()
	{
		calcDeltaTime();
	}

	void TimeSystem::SetTimeScale(float timeScale)
	{
		GM_ASSERT_RETURN(timeScale >= 0.f, "TimeScale은 0 이상이어야 합니다.");
		_timeScale = timeScale;
	}

	void TimeSystem::calcDeltaTime()
	{
		QueryPerformanceCounter(&_curFrequency);

		float diffFrequency = static_cast<float>(_curFrequency.QuadPart - _prevFrequency.QuadPart);
		_unscaledDeltaTime = diffFrequency / static_cast<float>(_cpuFrequency.QuadPart);
		_deltaTime = _unscaledDeltaTime * _timeScale;

		_prevFrequency = _curFrequency;
	}
}
