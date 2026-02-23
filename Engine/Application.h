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
	#define APPLICATION gm::Application::GetInstance()

	class Input;
	class Time;
	class SceneManager;

	class Application
	{
	private:
		Application();
		~Application();

	public:
		static Application& GetInstance()
		{
			static Application instance;
			return instance;
		}

	public:
		void				Initialize(HWND hWnd, uint32_t width, uint32_t height);
		void				Run();

		void				Update();
		void				LateUpdate();
		void				Render();

	public:
		Input&				GetInput() { return *_input; }
		Time&				GetTime() { return *_time; }
		SceneManager&		GetSceneManager() { return *_sceneManager; }

		const Input&		GetInput()        const { return *_input; }
		const Time&			GetTime()         const { return *_time; }
		const SceneManager& GetSceneManager() const { return *_sceneManager; }

	private:
		void				initializeWindow(HWND hWnd, uint32_t width, uint32_t height);
		void				createBackDC();
		void				initializeSubSystem();

	private:
		HWND							_hWnd = nullptr;
		HDC								_hDC = nullptr;

		HDC								_backHDC = nullptr;
		HBITMAP							_backBuffer = nullptr;
				
		uint32_t 						_width = 0;
		uint32_t 						_height = 0;

		std::unique_ptr<Input>			_input;
		std::unique_ptr<Time>			_time;
		std::unique_ptr<SceneManager>	_sceneManager;
	};
}


