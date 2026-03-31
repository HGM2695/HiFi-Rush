#pragma once

#define NOMINMAX
#include "Windows.h"

struct HDC__;
typedef struct HDC__* HDC;

namespace gm
{
	class Time
	{
	public:
		Time();

		void	Update();
		void	Render(HDC hDC);

		float	GetDeltaTime() const { return _deltaTime; }
		float	GetUnscaledDeltaTime() const { return _unscaledDeltaTime; }
		float	GetTimeScale() const { return _timeScale; }
		void	SetTimeScale(float timeScale);

	private:
		void calcDeltaTime();

	private:
		LARGE_INTEGER	_cpuFrequency{};
		LARGE_INTEGER	_prevFrequency{};
		LARGE_INTEGER	_curFrequency{};
		float			_deltaTime{};
		float			_unscaledDeltaTime{};
		float			_timeScale = 1.f;
	};
}


