#pragma once
#include "Component.h"

namespace gm
{
    class Transform : public Component
    {
    public:
        Transform();
        virtual ~Transform();

        void            SetPosition(const Vector2& position) { _position = position; }
        void            SetX(float x) { _position.x = x; }
        void            SetY(float y) { _position.y = y; }
        void            SetForward(const Vector2& forward) { _forward = forward; }

        void            Translate(const Vector2& delta) { _position += delta; }
        void            Translate(float dx, float dy) { _position.x += dx; _position.y += dy; }
        void            TranslateX(float dx) { _position.x += dx; }
        void            TranslateY(float dy) { _position.y += dy; }

        Vector2         GetPosition() const { return _position; }
        float           GetX() const { return _position.x; }
        float           GetY() const { return _position.y; }
        Vector2         GetForward() const { return _forward; }

    private:
        Vector2   _position{};
        Vector2   _forward{ 1.f, 0.f };
    };
}
