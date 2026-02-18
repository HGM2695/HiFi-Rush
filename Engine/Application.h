#pragma once

#include <memory>

struct HWND__;
typedef struct HWND__* HWND;
struct HDC__;
typedef struct HDC__* HDC;

namespace gm
{
	class Input;

	class Application
	{
	public:
		Application();
		~Application();

	public:
		void			Initialize(HWND hWnd);
		void			Run();

		void			Update();
		void			LateUpdate();
		void			Render();

	public:
		const Input*	GetInput() const { return _input.get(); }

	private:
		HWND					_hWnd = nullptr;
		HDC						_hDC = nullptr;
		std::unique_ptr<Input>	_input;
	};
}


