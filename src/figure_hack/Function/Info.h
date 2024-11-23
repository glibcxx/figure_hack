#pragma once

#include "figure_hack/CommonTypes.h"
#include <mc/world/level/BlockSource.h>
#include <optional>


namespace fh {

BlockInfo blockInfoAtPos(BlockSource& region, const BlockPos& pos);

std::optional<CircuitInfo> circuitInfoAtPos(BlockSource& region, const BlockPos& pos);

std::optional<ActorInfo> actorInfo(const Actor* except, BlockSource& region, const Vec3& from, const Vec3& to);

} // namespace fh