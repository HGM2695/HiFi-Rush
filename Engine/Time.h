#pragma once

#include "Windows.h"

struct HDC__;
typedef struct HDC__* HDC;

namespace gm
{
	class Time
	{
	public:
		void	Initialize();
		void	Update();
		void	Render(HDC hDC);

		float	GetDeltaTime() { return _deltaTime; }

	private:
		void calcDeltaTime();

	private:
		LARGE_INTEGER	_cpuFrequency;
		LARGE_INTEGER	_prevFrequency;
		LARGE_INTEGER	_curFrequency;
		float			_deltaTime;
	};
}


