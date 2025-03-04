#pragma once

#include <mc/world/level/BlockSource.h>

#include "figure_hack/CommonTypes.h"

namespace fh {

BlockInfo blockInfoAtPos(BlockSource& region, const BlockPos& pos);

std::optional<CircuitInfo> circuitInfoAtPos(BlockSource& region, const BlockPos& pos);

bool toggleActorInfo(BlockSource& region, ActorInfoMode mode);

std::string buildSpecialActorDbgString(Actor& actor);

} // namespace fh