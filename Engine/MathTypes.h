#pragma once

#include "Types.h"
#include <DirectXCollision.h>
#include <directxtk/SimpleMath.h>

#include <algorithm>
#include <cmath>

namespace gm
{
    using Vector2 = DirectX::SimpleMath::Vector2;
    using Vector3 = DirectX::SimpleMath::Vector3;
    using Vector4 = DirectX::SimpleMath::Vector4;
    using Matrix = DirectX::SimpleMath::Matrix;
    using Quaternion = DirectX::SimpleMath::Quaternion;
    using Color = DirectX::SimpleMath::Color;
    using Ray = DirectX::SimpleMath::Ray;
    using BoundingBox = DirectX::BoundingBox;
    using BoundingOrientedBox = DirectX::BoundingOrientedBox;
    using BoundingSphere = DirectX::BoundingSphere;
    using BoundingFrustum = DirectX::BoundingFrustum;

    struct Plane
    {
        Vector3 normal{};
        float   d = 0.f;

        float CalcY(float x, float z, float fallbackY = 0.f) const
        {
            if (std::abs(normal.y) <= 0.0001f)
                return fallbackY;

            return -(normal.x * x + normal.z * z + d) / normal.y;
        }
    };

    constexpr Color ColorFromRGBA(uint8 r, uint8 g, uint8 b, uint8 a = 255)
    {
        constexpr float invColor = 1.f / 255.f;
        return Color{ r * invColor, g * invColor, b * invColor, a * invColor };
    }

    inline float ConvertSRGBToLinear(float value)
    {
        return value <= 0.04045f ? value / 12.92f : std::pow((value + 0.055f) / 1.055f, 2.4f);
    }

    inline Color ConvertSRGBToLinear(const Color& color)
    {
        return Color{ ConvertSRGBToLinear(color.x), ConvertSRGBToLinear(color.y), ConvertSRGBToLinear(color.z), color.w };
    }

    namespace Colors
    {
        inline constexpr Color White = Color{ 1.f, 1.f, 1.f, 1.f };
        inline constexpr Color Black = Color{ 0.f, 0.f, 0.f, 1.f };
        inline constexpr Color Gray = Color{ 0.5f, 0.5f, 0.5f, 1.f };
        inline constexpr Color Red = Color{ 1.f, 0.f, 0.f, 1.f };
        inline constexpr Color Green = Color{ 0.f, 1.f, 0.f, 1.f };
        inline constexpr Color Blue = Color{ 0.f, 0.f, 1.f, 1.f };
        inline constexpr Color Yellow = Color{ 1.f, 1.f, 0.f, 1.f };
        inline constexpr Color Cyan = Color{ 0.f, 1.f, 1.f, 1.f };
        inline constexpr Color Magenta = Color{ 1.f, 0.f, 1.f, 1.f };
        inline constexpr Color Orange = ColorFromRGBA(255, 165, 0);
        inline constexpr Color Purple = Color{ 0.f, 0.f, 0.f, 1.f };
        inline constexpr Color Transparent = Color{ 0.f, 0.f, 0.f, 0.f };
    }
}
