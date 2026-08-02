#pragma once

#include "EngineCore.h"

namespace gm
{
    enum class TickGroup
    {
        GameLogic,
        Animation,
        Movement,
        PostPhysics,
        Attachment,
        Camera,
        Count
    };

    inline constexpr uint32 TickGroupToIndex(TickGroup group)
    {
        return static_cast<uint32>(group);
    }

    inline constexpr uint32 TickGroupCount = TickGroupToIndex(TickGroup::Count);
}
