#pragma once

struct HWND__;
typedef struct HWND__* HWND;
struct HDC__;
typedef struct HDC__* HDC;

namespace gm
{
	class Application
	{
	public:
		void	Initialize(HWND hWnd);
		void	Run();

		void	Update();
		void	LateUpdate();
		void	Render();

	private:
		HWND	_hWnd = nullptr;
		HDC		_hDC = nullptr;
	};
}


