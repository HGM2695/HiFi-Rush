#pragma once

#include "EngineCore.h"
#include "WindowsCore.h"
#include <string>

namespace gm
{
	struct WindowDesc
	{
		HINSTANCE		instance = nullptr;
		std::wstring	title = L"";
		std::wstring	className = L"";
		int				width = 0;
		int				height = 0;
		int				showCommand = 0;
	};

	class Window
	{
	public:
		bool	Initialize(const WindowDesc& desc);
		LRESULT MsgProc(HWND hWnd, UINT32 msg, WPARAM wParam, LPARAM lParam);

		HWND	GetHandle() const { return _hWnd; }
		uint32	GetWidth() const { return _width; }
		uint32	GetHeight() const { return _height; }

	private:
		static	LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		bool	RegistClass(HINSTANCE instance, const std::wstring& className);
		bool	CreateWindowHandle(uint32 width, uint32 height, HINSTANCE instance, const std::wstring& className, const std::wstring& title, int nCmdShow);

	private:
		HINSTANCE	instance;
		HWND		_hWnd{};
		uint32		_width{};
		uint32		_height{};
	};
}


