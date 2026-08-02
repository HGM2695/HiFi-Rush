#include "Input.h"
#include <Windows.h>

namespace gm
{
	constexpr int ASCII[static_cast<int>(KeyCode::Count)] =
	{
		0x1B,
		0x0D,
		0x20,
		0x09,
		0x08,

		0xA0,
		0xA1,
		0xA2,
		0xA3,
		0xA4,
		0xA5,

		0x25,
		0x27,
		0x26,
		0x28,

		0x2D,
		0x2E,
		0x24,
		0x23,
		0x21,
		0x22,

		0x70, 0x71, 0x72, 0x73,
		0x74, 0x75, 0x76, 0x77,
		0x78, 0x79, 0x7A, 0x7B,

		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',

		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',

		0x60, 0x61, 0x62, 0x63, 0x64,
		0x65, 0x66, 0x67, 0x68, 0x69,
		0x6B, 0x6D, 0x6A, 0x6F,
		0x0D, 0x6E,

		0xBD,
		0xBB,
		0xDB,
		0xDD,
		0xDC,
		0xBA,
		0xDE,
		0xBC,
		0xBE,
		0xBF,
		0xC0
	};

	constexpr int MOUSE_BUTTONS[static_cast<int>(MouseButton::Count)] =
	{
		VK_LBUTTON,
		VK_RBUTTON,
		VK_MBUTTON
	};

	Input::Input(HWND hWnd)
	{
		static_assert(
			ASCII[static_cast<int>(KeyCode::Count) - 1],
			"KeyCode에 추가했다면 ASCII에도 추가해주세요."
			);

		_hWnd = hWnd;

		_keyList.resize(static_cast<int>(KeyCode::Count));
		for (int i = 0; i < static_cast<int>(KeyCode::Count); ++i)
			_keyList[i] = { static_cast<KeyCode>(i), KeyState::None, false };

		_mouseList.resize(static_cast<int>(MouseButton::Count));
		for (int i = 0; i < static_cast<int>(MouseButton::Count); ++i)
			_mouseList[i] = { static_cast<MouseButton>(i), KeyState::None, false };

		updateMousePosition();
		_previousMousePosition = _mousePosition;
	}

	void Input::Tick()
	{
		if (isWindowFocused() == false)
		{
			setCursorVisible(true);
			clearInputState();
			return;
		}

		const bool restoreCursorLock = _cursorLocked && _cursorVisible;
		setCursorVisible(_cursorLocked == false);
		if (restoreCursorLock)
		{
			const Vector2 center = getClientCenter();
			_mousePosition = center;
			_previousMousePosition = center;
			_mouseDelta = {};
			moveCursorToClientPosition(center);
		}

		updateKeyState();
		updateMouseState();
		updateMousePosition();
	}

	void Input::SetCursorLocked(bool locked)
	{
		if (_cursorLocked == locked)
			return;

		_cursorLocked = locked;
		_mouseDelta = {};
		setCursorVisible(locked == false || isWindowFocused() == false);

		if (_cursorLocked)
		{
			const Vector2 center = getClientCenter();
			_mousePosition = center;
			_previousMousePosition = center;
			moveCursorToClientPosition(center);
			return;
		}

		updateMousePosition();
		_previousMousePosition = _mousePosition;
		_mouseDelta = {};
	}

	Vector2 Input::GetAxis2D(KeyCode right, KeyCode left, KeyCode up, KeyCode down) const
	{
		Vector2 v
		{
			GetAxis(right, left),
			GetAxis(up, down)
		};

		if (v.LengthSquared() > 0.f)
			v.Normalize();

		return v;
	}

	bool Input::isWindowFocused() const
	{
		const HWND foregroundWindow = GetForegroundWindow();
		return foregroundWindow == _hWnd || IsChild(_hWnd, foregroundWindow);
	}

	void Input::clearInputState()
	{
		for (Key& key : _keyList)
		{
			key._keyState = KeyState::None;
			key._pressed = false;
		}

		for (Mouse& mouse : _mouseList)
		{
			mouse._keyState = KeyState::None;
			mouse._pressed = false;
		}

		_mouseDelta = {};
	}

	void Input::updateKeyState()
	{
		const int size = static_cast<int>(_keyList.size());
		for (int i = 0; i < size; ++i)
		{
			if (GetAsyncKeyState(ASCII[i]) & 0x8000)
			{
				if (_keyList[i]._pressed)
					_keyList[i]._keyState = KeyState::Repeat;
				else
					_keyList[i]._keyState = KeyState::Down;

				_keyList[i]._pressed = true;
			}
			else
			{
				if (_keyList[i]._pressed)
					_keyList[i]._keyState = KeyState::Up;
				else
					_keyList[i]._keyState = KeyState::None;

				_keyList[i]._pressed = false;
			}
		}
	}

	void Input::updateMouseState()
	{
		const int size = static_cast<int>(_mouseList.size());
		for (int i = 0; i < size; ++i)
		{
			if (GetAsyncKeyState(MOUSE_BUTTONS[i]) & 0x8000)
			{
				if (_mouseList[i]._pressed)
					_mouseList[i]._keyState = KeyState::Repeat;
				else
					_mouseList[i]._keyState = KeyState::Down;

				_mouseList[i]._pressed = true;
			}
			else
			{
				if (_mouseList[i]._pressed)
					_mouseList[i]._keyState = KeyState::Up;
				else
					_mouseList[i]._keyState = KeyState::None;

				_mouseList[i]._pressed = false;
			}
		}
	}

	void Input::updateMousePosition()
	{
		POINT pt{};
		GetCursorPos(&pt);
		ScreenToClient(_hWnd, &pt);

		_mousePosition = { static_cast<float>(pt.x), static_cast<float>(pt.y) };

		if (_cursorLocked)
		{
			const Vector2 center = getClientCenter();
			_mouseDelta = _mousePosition - center;
			_mousePosition = center;
			_previousMousePosition = center;
			moveCursorToClientPosition(center);
			return;
		}

		_mouseDelta = _mousePosition - _previousMousePosition;
		_previousMousePosition = _mousePosition;
	}

	void Input::setCursorVisible(bool visible)
	{
		if (_cursorVisible == visible)
			return;

		if (visible)
		{
			while (ShowCursor(TRUE) < 0)
			{
			}
		}
		else
		{
			while (ShowCursor(FALSE) >= 0)
			{
			}
		}

		_cursorVisible = visible;
	}

	Vector2 Input::getClientCenter() const
	{
		RECT clientRect{};
		GetClientRect(_hWnd, &clientRect);
		return Vector2
		{
			static_cast<float>(clientRect.right - clientRect.left) * 0.5f,
			static_cast<float>(clientRect.bottom - clientRect.top) * 0.5f,
		};
	}

	void Input::moveCursorToClientPosition(const Vector2& position) const
	{
		POINT screenPosition{ static_cast<LONG>(position.x), static_cast<LONG>(position.y) };
		ClientToScreen(_hWnd, &screenPosition);
		SetCursorPos(screenPosition.x, screenPosition.y);
	}
}
