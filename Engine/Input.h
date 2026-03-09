#pragma once
#include <vector>
#include "Vector2.h"

namespace gm
{
	enum class KeyState
	{
        None,
		Down,		// 처음 눌림
		Pressed,	// 유지
		Up,			// 뗌
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
            KeyCode					_keyCode{};
            KeyState				_keyState{ KeyState::None };
            bool					_pressed{};
        };

        void			Initialize();
        void			Update();

        bool            IsKeyUp(KeyCode code) const { return getKey(code)._keyState == KeyState::Up; }
        bool            IsKeyDown(KeyCode code) const { return getKey(code)._keyState == KeyState::Down; }
        bool            IsKeyRepeat(KeyCode code) const { return getKey(code)._pressed == true; }

        float           GetAxis(KeyCode positive, KeyCode negative) const { return (IsKeyRepeat(positive) ? 1.f : 0.f) - (IsKeyRepeat(negative) ? 1.f : 0.f); }
        math::Vector2   GetAxis2D(KeyCode right, KeyCode left, KeyCode down, KeyCode up) const;

        float           GetMoveAxisX() const { return GetAxis(KeyCode::Right, KeyCode::Left); }
        float           GetMoveAxisY() const { return GetAxis(KeyCode::Down, KeyCode::Up); }
        math::Vector2   GetMoveAxisXY() const { return GetAxis2D(KeyCode::Right, KeyCode::Left, KeyCode::Down, KeyCode::Up); }

    private:
        const Key&      getKey(KeyCode code) const { return _keyList[static_cast<size_t>(code)]; }
		void			updateKeyListState();

	private:
		std::vector<Key>			_keyList;
	};
}
