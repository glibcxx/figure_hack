#pragma once

#include <mc/world/level/BlockPos.h>

namespace fh {

class CPUVisualize {
public:
    // add or remove a pos, return true if added, else false
    static bool switchPos(DimensionType dimType, const BlockPos& pos);

    static bool tryRemovePos(DimensionType dimType, const BlockPos& pos);

    static void clearPos();
};

} // namespace fh
