#pragma once

#include <mc/world/level/BlockPos.h>
#include <mc/world/level/BlockSource.h>

namespace fh {

class CPUVisualize {
public:
    // add or remove a pos, return true if added, else false
    static bool switchPos(BlockSource& region, const BlockPos& pos);

    static bool tryRemovePos(BlockSource& region, const BlockPos& pos);

    static void clearPos(Level &level);
};

} // namespace fh
