#include "Info.h"
#include "mc/world/actor/Actor.h"
#include <mc/world/actor/player/Player.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/dimension/Dimension.h>
#include <mc/world/phys/AABB.h>
#include <mc/world/redstone/circuit/CircuitSystem.h>


namespace fh {
BlockInfo blockInfoAtPos(BlockSource& region, const BlockPos& pos) {
    const Block& block = region.getBlock(pos);
    return {block.getName().getString()};
}

std::optional<CircuitInfo> circuitInfoAtPos(BlockSource& region, const BlockPos& pos) {
    CircuitSystem&        system = region.getDimension().getCircuitSystem();
    CircuitSceneGraph&    graph  = system.mSceneGraph;
    BaseCircuitComponent* comp   = graph.getBaseComponent(pos);
    if (comp) {
        return {
            {.typeId   = comp->getCircuitComponentType(),
             .rawPtr   = comp,
             .sources  = comp->mSources,
             .strength = comp->getStrength()}
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
        if ((actorBB.contains(from) || actorBB.clip(from, to)) // from -> to 与 actorBB 有交点
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
