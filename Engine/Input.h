#pragma once
#include <vector>

namespace gm
{
	enum class KeyState
	{
		Down,		// 처음 눌림
		Pressed,	// 유지
		Up,			// 뗌
		None,
		Count
	};

    enum class KeyCode : uint16_t
    {
        // --- Control / System ---
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

        // --- Arrow / Navigation ---
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

        // --- Function keys ---
        F1, F2, F3, F4, 
        F5, F6, F7, F8, 
        F9, F10, F11, F12,

        // --- Number row (top) ---
        D0, D1, D2, D3, D4, D5, D6, D7, D8, D9,

        // --- Alphabet ---
        A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

        // --- Numpad ---
        Num0, Num1, Num2, Num3, Num4,
        Num5, Num6, Num7, Num8, Num9,
        NumAdd, NumSub, NumMul, NumDiv,
        NumEnter, NumDot,

        // --- Common symbols ---
        Minus,      // -
        Equals,     // =
        LBracket,   // [
        RBracket,   // ]
        Backslash,  // '\'
        Semicolon,  // ;
        Apostrophe, // '
        Comma,      // ,
        Period,     // .
        Slash,      // /
        Grave,      // `

        Count
    };


	class Input
	{
	public:
		struct Key
		{
			KeyCode					_keyCode;
			KeyState				_keyState;
			bool					_pressed;
		};

		void						Initialize();
		void						Update();

		bool						IsKeyDown(KeyCode code) const { return _keyList[static_cast<int>(code)]._keyState == KeyState::Down; }
		bool						IsKeyPressed(KeyCode code) const { return _keyList[static_cast<int>(code)]._keyState == KeyState::Pressed; }
		bool						IsKeyUp(KeyCode code) const { return _keyList[static_cast<int>(code)]._keyState == KeyState::Up; }

	private:
		void						UpdateKeyListState();

	private:
		std::vector<Key>			_keyList;
	};
}
