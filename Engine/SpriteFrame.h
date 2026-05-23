#pragma once

#include "Rect.h"

namespace gm
{
    struct SpriteFrame
    {
        IntRect sourceRect{};
        float   duration = 0.1f;

        int32 Left() const { return sourceRect.left; }
        int32 Top() const { return sourceRect.top; }
        int32 Width() const { return sourceRect.width; }
        int32 Height() const { return sourceRect.height; }
    };
}
