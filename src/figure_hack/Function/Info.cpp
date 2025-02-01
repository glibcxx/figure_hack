#include "Info.h"

#include <ll/api/i18n/I18n.h>
#include <ll/api/memory/Hook.h>
#include <mc/deps/core/math/Vec2.h>
#include <mc/deps/ecs/gamerefs_entity/EntityContext.h>
#include <mc/entity/components/ActorUniqueIDComponent.h>
#include <mc/network/SpatialActorNetworkData.h>
#include <mc/server/commands/Command.h>
#include <mc/server/commands/CommandUtils.h>
#include <mc/world/Minecraft.h>
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

#include "figure_hack/Utils/TextMarker.h"

namespace fh {

using namespace ll::i18n_literals;

BlockInfo blockInfoAtPos(BlockSource& region, const BlockPos& pos) {
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

    // TextMarker::addText(region, fmt::format("{}", retActor->getPosition().toString()), retActor->getPosition());

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

std::unordered_map<Actor*, TextMarker::TextHandle> actors{};

void _getActorInfo(Actor& actor) {
    if (actor.getActorIdentifier().getNamespace() != "fh" && !actor.isPlayer() && !actor.isRemoved()) {
        const ::Vec3& pos  = actor.getPosition();
        const AABB&   aabb = actor.getAABB();
        auto          uniqueIdComp =
            actor.mEntityContext.get().mEnTTRegistry.try_get<ActorUniqueIDComponent>(actor.mEntityContext.get().mEntity
            );
        std::string dbgStr = fmt::format(
            "{}\n"
            "UniqueId: {}\n"
            "RuntimeId: {}\n"
            "Pos: {}\n"
            "PosPrev: {}\n"
            "Vel: {}\n"
            "Rot: {}\n"
            "Health: {}\n",
            actor.getTypeName(),
            uniqueIdComp ? std::to_string(uniqueIdComp->mUnkf9218a.as<int64_t>()) : "None",
            actor.getRuntimeID().rawID,
            pos.toString(),
            actor.getPosPrev().toString(),
            actor.getPosDelta().toString(),
            actor.getRotation(),
            actor.getHealth()
        );
        auto it = actors.find(&actor);
        if (it == actors.end()) {
            actors.emplace(
                &actor,
                TextMarker::addText(actor.getDimensionBlockSource(), dbgStr, Vec3{pos.x, aabb.max.y, pos.z})
            );
        } else {
            it->second.change(dbgStr, Vec3{pos.x, aabb.max.y, pos.z});
        }
    }
}

LL_TYPE_INSTANCE_HOOK(
    ActorTickDebug_hook,
    ll::memory::HookPriority::Normal,
    SpatialActorNetworkData,
    &SpatialActorNetworkData::sendUpdate,
    void,
    bool forceTeleport,
    bool forceMoveLocalEntity,
    bool forceAbsoluteMovement
) {
    this->origin(forceTeleport, forceMoveLocalEntity, forceAbsoluteMovement);
    Actor& actor = this->mUnk3d576d.as<Actor&>();
    _getActorInfo(actor);
}

LL_TYPE_INSTANCE_HOOK(
    ActorTickDebug2_hook,
    ll::memory::HookPriority::Normal,
    Actor,
    &Actor::tick,
    bool,
    ::BlockSource& region
) {
    bool res = this->origin(region);
    _getActorInfo(*this);
    return res;
}

LL_TYPE_INSTANCE_HOOK(ActorTickDebug_ActorLoaded_hook, ll::memory::HookPriority::Normal, Actor, &Actor::reload, void) {
    this->origin();
    _getActorInfo(*this);
}

LL_TYPE_INSTANCE_HOOK(
    ActorTickDebug_ActorRemoved_hook,
    ll::memory::HookPriority::Normal,
    Actor,
    &Actor::$remove,
    void
) {
    actors.erase(this);
    this->origin();
}

LL_TYPE_INSTANCE_HOOK(ActorTickDebug3_hook, ll::memory::HookPriority::Normal, Minecraft, &Minecraft::update, bool) {
    if (this->getSimPaused()) {
        for (auto&& e : this->getLevel()->getRuntimeActorList()) {
            _getActorInfo(*e);
        }
    }
    return this->origin();
}

bool toggleActorInfo(BlockSource& region) {
    static bool isOn = false;
    isOn             = !isOn;
    if (isOn) {
        ll::memory::HookRegistrar<ActorTickDebug_ActorLoaded_hook>::hook();
        ll::memory::HookRegistrar<ActorTickDebug_hook>::hook();
        ll::memory::HookRegistrar<ActorTickDebug2_hook>::hook();
        ll::memory::HookRegistrar<ActorTickDebug3_hook>::hook();
        ll::memory::HookRegistrar<ActorTickDebug_ActorRemoved_hook>::hook();
    } else {
        ll::memory::HookRegistrar<ActorTickDebug_ActorLoaded_hook>::unhook();
        ll::memory::HookRegistrar<ActorTickDebug_hook>::unhook();
        ll::memory::HookRegistrar<ActorTickDebug2_hook>::unhook();
        ll::memory::HookRegistrar<ActorTickDebug3_hook>::unhook();
        ll::memory::HookRegistrar<ActorTickDebug_ActorRemoved_hook>::unhook();
        actors.clear();
    }
    return isOn;
}


} // namespace fh
