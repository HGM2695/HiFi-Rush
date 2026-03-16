#pragma once
#include "Component.h"
#include "Vector2.h"

namespace gm
{
    class Transform : public Component
    {
    public:
        Transform();
        virtual ~Transform();

        void            SetPosition(const math::Vector2& position) { _position = position; }
        void            SetX(float x) { _position._x = x; }
        void            SetY(float y) { _position._y = y; }
        void            SetForward(const math::Vector2& forward) { _forward = forward; }

        void            Translate(const math::Vector2& delta) { _position += delta; }
        void            Translate(float dx, float dy) { _position._x += dx; _position._y += dy; }
        void            TranslateX(float dx) { _position._x += dx; }
        void            TranslateY(float dy) { _position._y += dy; }

        math::Vector2   GetPosition() const { return _position; }
        float           GetX() const { return _position._x; }
        float           GetY() const { return _position._y; }
        math::Vector2   GetForward() const { return _forward; }

    private:
        math::Vector2   _position{};
        math::Vector2   _forward{ 1.f, 0.f };
    };
}
