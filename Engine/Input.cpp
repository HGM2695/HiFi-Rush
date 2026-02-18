#include "Input.h"
#include <Windows.h>

namespace gm
{
    constexpr int ASCII[static_cast<int>(KeyCode::Count)] =
    {
        // --- Control / System ---
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

        // --- Arrow / Navigation ---
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

        // --- Function keys ---
        0x70, 0x71, 0x72, 0x73,
        0x74, 0x75, 0x76, 0x77,
        0x78, 0x79, 0x7A, 0x7B,

        // --- Number row (top) ---
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',

        // --- Alphabet ---
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',

        // --- Numpad ---
        0x60, 0x61, 0x62, 0x63, 0x64,
        0x65, 0x66, 0x67, 0x68, 0x69,
        0x6B, 0x6D, 0x6A, 0x6F,
        0x0D, 0x6E,

        // --- Common symbols ---
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

	void Input::Initialize()
	{
        static_assert(
            ASCII[static_cast<int>(KeyCode::Count) - 1],
            "KeyCode에 추가했다면 ASCII에도 추가해주세요."
            );

		_keyList.resize(static_cast<int>(KeyCode::Count));
		for (int i = 0; i < static_cast<int>(KeyCode::Count); i++)
			_keyList[i] = { static_cast<KeyCode>(i), KeyState::None, false };
	}

	void Input::Update()
	{
		updateKeyListState();
	}

	void Input::updateKeyListState()
	{
		int Size = static_cast<int>(_keyList.size());
		for (int i = 0; i < Size; i++)
		{
			if (GetAsyncKeyState(ASCII[i]) & 0x8000)
			{
				if (_keyList[i]._pressed == true)
					_keyList[i]._keyState = KeyState::Pressed;
				else
					_keyList[i]._keyState = KeyState::Down;

				_keyList[i]._pressed = true;
			}
			else
			{
				if (_keyList[i]._pressed == true)
					_keyList[i]._keyState = KeyState::Up;
				else
					_keyList[i]._keyState = KeyState::None;

				_keyList[i]._pressed = false;
			}
		}
	}

}
