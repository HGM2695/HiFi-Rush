#pragma once

#include <memory>

struct HWND__;
typedef struct HWND__* HWND;
struct HDC__;
typedef struct HDC__* HDC;
struct HBITMAP__;
typedef struct HBITMAP__* HBITMAP;

namespace gm
{
	class Input;
	class Time;

	class Application
	{
	public:
		Application();
		~Application();

	public:
		void			Initialize(HWND hWnd, uint32_t width, uint32_t height);
		void			Run();

		void			Update();
		void			LateUpdate();
		void			Render();

	public:
		const Input*	GetInput() const { return _input.get(); }

	private:
		void			initializeWindow(HWND hWnd, uint32_t width, uint32_t height);
		void			createBackDC();
		void			initializeSubSystem();

	private:
		HWND					_hWnd = nullptr;
		HDC						_hDC = nullptr;

		HDC						_backHDC = nullptr;
		HBITMAP					_backBuffer = nullptr;

		uint32_t 				_width = 0;
		uint32_t 				_height = 0;

		std::unique_ptr<Input>	_input;
		std::unique_ptr<Time>	_time;
	};
}


