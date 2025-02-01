#pragma once

#include <mc/world/level/BlockSource.h>

#include "figure_hack/CommonTypes.h"

namespace fh {

BlockInfo blockInfoAtPos(BlockSource& region, const BlockPos& pos);

std::optional<CircuitInfo> circuitInfoAtPos(BlockSource& region, const BlockPos& pos);

std::optional<ActorInfo> actorInfo(const Actor* except, BlockSource& region, const Vec3& from, const Vec3& to);

bool toggleActorInfo(BlockSource& region, ActorInfoMode mode);

} // namespace fh