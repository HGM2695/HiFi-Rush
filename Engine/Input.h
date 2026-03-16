#pragma once

#include <vector>
#include "Vector2.h"

struct HWND__;
typedef struct HWND__* HWND;

namespace gm
{
	enum class KeyState
	{
		None,
		Down,
		Pressed,
		Up,
		Count
	};

	enum class KeyCode : uint16_t
	{
		Escape,
		Enter,
		Space,
		Tab,
		Backspace,

		LeftShift,
		RightShift,
		LeftCtrl,
		RightCtrl,
		LeftAlt,
		RightAlt,

		Left,
		Right,
		Up,
		Down,

		Insert,
		Delete,
		Home,
		End,
		PageUp,
		PageDown,

		F1, F2, F3, F4,
		F5, F6, F7, F8,
		F9, F10, F11, F12,

		D0, D1, D2, D3, D4, D5, D6, D7, D8, D9,

		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

		Num0, Num1, Num2, Num3, Num4,
		Num5, Num6, Num7, Num8, Num9,
		NumAdd, NumSub, NumMul, NumDiv,
		NumEnter, NumDot,

		Minus,
		Equals,
		LBracket,
		RBracket,
		Backslash,
		Semicolon,
		Apostrophe,
		Comma,
		Period,
		Slash,
		Grave,

		Count
	};

	enum class MouseButton : uint8_t
	{
		Left,
		Right,
		Middle,

		Count
	};

	class Input
	{
	public:
		struct Key
		{
			KeyCode		_keyCode{};
			KeyState	_keyState{ KeyState::None };
			bool		_pressed{};
		};

		struct Mouse
		{
			MouseButton	_button{};
			KeyState	_keyState{ KeyState::None };
			bool		_pressed{};
		};

		// Keyboard
		void			Initialize(HWND hWnd);
		void			Update();

		bool			IsKeyUp(KeyCode code) const { return getKey(code)._keyState == KeyState::Up; }
		bool			IsKeyDown(KeyCode code) const { return getKey(code)._keyState == KeyState::Down; }
		bool			IsKeyRepeat(KeyCode code) const { return getKey(code)._pressed == true; }

		float			GetAxis(KeyCode positive, KeyCode negative) const { return (IsKeyRepeat(positive) ? 1.f : 0.f) - (IsKeyRepeat(negative) ? 1.f : 0.f); }
		math::Vector2	GetAxis2D(KeyCode right, KeyCode left, KeyCode down, KeyCode up) const;

		float			GetMoveAxisX() const { return GetAxis(KeyCode::Right, KeyCode::Left); }
		float			GetMoveAxisY() const { return GetAxis(KeyCode::Down, KeyCode::Up); }
		math::Vector2	GetMoveAxisXY() const { return GetAxis2D(KeyCode::Right, KeyCode::Left, KeyCode::Down, KeyCode::Up); }

		// Mouse
		bool			IsMouseUp(MouseButton button) const { return getMouse(button)._keyState == KeyState::Up; }
		bool			IsMouseDown(MouseButton button) const { return getMouse(button)._keyState == KeyState::Down; }
		bool			IsMouseRepeat(MouseButton button) const { return getMouse(button)._pressed == true; }

		math::Vector2	GetMousePosition() const { return _mousePosition; }
		math::Vector2	GetMouseDelta() const { return _mouseDelta; }

	private:
		// Keyboard
		const Key&		getKey(KeyCode code) const { return _keyList[static_cast<size_t>(code)]; }
		void			updateKeyListState();

		// Mouse
		const Mouse&	getMouse(MouseButton button) const { return _mouseList[static_cast<size_t>(button)]; }
		void			updateMouseState();
		void			updateMousePosition();

	private:
		std::vector<Key>	_keyList;

		HWND				_hWnd = nullptr;
		std::vector<Mouse>	_mouseList;
		math::Vector2		_mousePosition{};
		math::Vector2		_previousMousePosition{};
		math::Vector2		_mouseDelta{};
	};
}
