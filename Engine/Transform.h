#pragma once
#include "Component.h"

namespace gm
{
    class Transform : public Component
    {
    public:
        Transform();
        virtual ~Transform();

        void            SetPosition(const Vector3& position) { _position = position; }
        void            SetPosition(const Vector2& position) { _position = Vector3(position.x, position.y, _position.z); }
        void            SetPosition2D(const Vector2& position) { SetPosition(position); }
        void            SetX(float x) { _position.x = x; }
        void            SetY(float y) { _position.y = y; }
        void            SetZ(float z) { _position.z = z; }
        void            SetRotation(const Quaternion& rotation) { _rotation = rotation; }
        void            SetRotationEuler(const Vector3& eulerRadians);
        void            SetRotationZ(float radians);
        void            SetScale(const Vector3& scale) { _scale = scale; }
        void            SetScale(const Vector2& scale) { _scale = Vector3(scale.x, scale.y, _scale.z); }
        void            SetWorldMatrix(const Matrix& worldMatrix);
        void            Translate(const Vector3& delta) { _position += delta; }
        void            Translate(const Vector2& delta) { _position.x += delta.x; _position.y += delta.y; }
        void            Translate(float dx, float dy) { _position.x += dx; _position.y += dy; }
        void            TranslateX(float dx) { _position.x += dx; }
        void            TranslateY(float dy) { _position.y += dy; }
        void            TranslateZ(float dz) { _position.z += dz; }

        Vector3         GetPosition() const { return _position; }
        Vector2         GetPosition2D() const { return Vector2(_position.x, _position.y); }
        Quaternion      GetRotation() const { return _rotation; }
        Vector3         GetScale() const { return _scale; }
        float           GetX() const { return _position.x; }
        float           GetY() const { return _position.y; }
        float           GetZ() const { return _position.z; }
        Matrix          GetWorldMatrix() const;

    private:
        Vector3         _position{};
        Quaternion      _rotation{ 0.f, 0.f, 0.f, 1.f };
        Vector3         _scale{ 1.f, 1.f, 1.f };
    };
}
