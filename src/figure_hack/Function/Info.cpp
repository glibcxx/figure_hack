#include "Info.h"

#include <ll/api/i18n/I18n.h>
#include <mc/deps/core/math/Vec2.h>
#include <mc/server/commands/Command.h>
#include <mc/server/commands/CommandUtils.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/ActorDefinitionIdentifier.h>
#include <mc/world/actor/ActorFactory.h>
#include <mc/world/actor/ActorFactoryData.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/dimension/Dimension.h>
#include <mc/world/phys/AABB.h>
#include <mc/world/phys/AABBHitResult.h>
#include <mc/world/redstone/circuit/CircuitSceneGraph.h>
#include <mc/world/redstone/circuit/CircuitSystem.h>


namespace fh {
BlockInfo blockInfoAtPos(BlockSource& region, const BlockPos& pos) {
    using namespace ll::i18n_literals;
    const Block& block = region.getBlock(pos);
    return {"{}"_tr(block.buildDescriptionName())};
}

std::optional<CircuitInfo> circuitInfoAtPos(BlockSource& region, const BlockPos& pos) {
    CircuitSystem&     system = region.getDimension().getCircuitSystem();
    CircuitSceneGraph& graph  = system.mSceneGraph;
    auto               comp   = graph.mAllComponents.find(pos);
    if (comp != graph.mAllComponents.end()) {
        return {
            CircuitInfo{
                        .typeId   = comp->second->getCircuitComponentType(),
                        .rawPtr   = comp->second.get(),
                        .sources  = comp->second->mSources.get(),
                        .strength = comp->second->getStrength()
            }
        };
    } else {
        return std::nullopt;
    }
}

std::optional<ActorInfo> actorInfo(const Actor* owner, BlockSource& region, const Vec3& from, const Vec3& to) {
    auto actors = region.fetchEntities(
        owner,
        AABB{from, from}.cloneAndGrow(0.125f).cloneAndExpandAlongDirection(to - from),
        true,
        false
    );
    Actor* retActor = nullptr;
    for (auto&& actor : actors) {
        const AABB& actorBB = actor->getAABB();
        if ((actorBB.contains(from) || (actorBB.clip(from, to).mUnk8b4661.as<bool>())
            ) // from -> to 与 actorBB 有交点, 我也不知道mUnk8b4661是什么，但试了一遍好像能跑
            && (!retActor || from.distanceToSqr(actor->getPosition()) < from.distanceToSqr(retActor->getPosition())
            )) { // 只取最近的一个
            retActor = actor;
        }
    }

    return retActor ? std::optional<ActorInfo>{
                          {
                           .typeId    = retActor->getTypeName(),
                           .runtimeId = retActor->getRuntimeID(),
                           .pos       = retActor->getPosition(),
                           .posPrev   = retActor->getPosPrev(),
                           .velocity  = retActor->getPosDelta(),
                           }}
                    : std::nullopt;
}


} // namespace fh
