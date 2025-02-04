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

#include "figure_hack/CommonTypes.h"
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

std::unordered_map<Actor*, TextMarker::TextObj> g_actors{};

ActorInfoMode g_currentMode = ActorInfoMode::overall;

std::string _Vec3AsString(const Vec3& p) { return fmt::format("({:+.15E}, {:+.15E}, {:+.15E})", p.x, p.y, p.z); }
std::string _Vec2AsString(const Vec2& p) { return fmt::format("({:+.15E}, {:+.15E})", p.x, p.y); }

std::string _buildActorDbgString(Actor& actor) {
    const Vec3& pos = actor.getPosition();
    auto        uniqueIdComp =
        actor.mEntityContext.get().mEnTTRegistry.try_get<ActorUniqueIDComponent>(actor.mEntityContext.get().mEntity);
    switch (g_currentMode) {
    default:
    case ActorInfoMode::overall:
        return fmt::format(
            "{}\n"
            "UniqueId: {}\n"
            "RuntimeId: {}\n"
            "Pos: {}\n"
            "PosPrev: {}\n"
            "Vel: {}\n"
            "Rot: {}\n"
            "Health: {}",
            actor.getTypeName(),
            uniqueIdComp ? std::to_string(uniqueIdComp->mUnkf9218a.as<int64_t>()) : "None",
            actor.getRuntimeID().rawID,
            pos.toString(),
            actor.getPosPrev().toString(),
            actor.getPosDelta().toString(),
            actor.getRotation(),
            actor.getHealth()
        );
    case ActorInfoMode::type:
        return fmt::format(
            "{}\n"
            "Variant: {}\n"
            "UniqueId: {}\n"
            "RuntimeId: {}",
            actor.getTypeName(),
            actor.getVariant(),
            uniqueIdComp ? std::to_string(uniqueIdComp->mUnkf9218a.as<int64_t>()) : "None",
            actor.getRuntimeID().rawID
        );
    case ActorInfoMode::movement:
        return fmt::format(
            "1. Pos 2. PosPrev  3. Vel 4. |Vel| 5. Rot\n"
            "1. {}\n"
            "2. {}\n"
            "3. {}\n"
            "4. {:+.15E}\n"
            "5. {}",
            _Vec3AsString(pos),
            _Vec3AsString(actor.getPosPrev()),
            _Vec3AsString(actor.getPosDelta()),
            actor.getPosDelta().length(),
            _Vec2AsString(actor.getRotation())
        );
    case ActorInfoMode::status:
        return fmt::format(
            "Global: {: <}   Autonomous: {: <}  \n"
            "Alive: {: <}    Baby: {: <}        \n"
            "Bribed: {: <}   Angry: {: <}       \n"
            "Climbing: {: <} Moving: {: <}      \n"
            "OnFire: {: <}   OnGround: {: <}    \n"
            "Health: {: <}   FallDistance: {: <}",
            actor.isGlobal(),
            actor.isAutonomous(),
            actor.isAlive(),
            actor.isBaby(),
            actor.isBribed(),
            actor.isAngry(),
            actor.isClimbing(),
            actor.isMoving(),
            actor.isOnFire(),
            actor.isOnGround(),
            actor.getHealth(),
            actor.getFallDistance()
        );
    }
}

void _getActorInfo(Actor& actor) {
    if (actor.getActorIdentifier().getNamespace() != "fh" && !actor.isPlayer() && !actor.isRemoved()) {
        const Vec3& pos    = actor.getPosition();
        const AABB& aabb   = actor.getAABB();
        std::string dbgStr = _buildActorDbgString(actor);
        auto        it     = g_actors.find(&actor);
        if (it == g_actors.end()) {
            g_actors.try_emplace(
                &actor,
                TextMarker::addText(actor.getDimensionBlockSource(), dbgStr, Vec3{pos.x, aabb.max.y, pos.z}, true)
            );
        } else {
            it->second.change(dbgStr, Vec3{pos.x, aabb.max.y, pos.z}, true);
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
    g_actors.erase(this);
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

bool toggleActorInfo(BlockSource& region, ActorInfoMode mode) {
    static bool isOn = false, toggle = false;
    if (mode == ActorInfoMode::toggle) {
        toggle = true;
        isOn   = !isOn;
    } else {
        toggle        = isOn == false;
        isOn          = true;
        g_currentMode = mode;
    }
    if (toggle && isOn) {
        ll::memory::HookRegistrar<ActorTickDebug_ActorLoaded_hook>::hook();
        ll::memory::HookRegistrar<ActorTickDebug_hook>::hook();
        ll::memory::HookRegistrar<ActorTickDebug2_hook>::hook();
        ll::memory::HookRegistrar<ActorTickDebug3_hook>::hook();
        ll::memory::HookRegistrar<ActorTickDebug_ActorRemoved_hook>::hook();
    } else if (toggle) {
        ll::memory::HookRegistrar<ActorTickDebug_ActorLoaded_hook>::unhook();
        ll::memory::HookRegistrar<ActorTickDebug_hook>::unhook();
        ll::memory::HookRegistrar<ActorTickDebug2_hook>::unhook();
        ll::memory::HookRegistrar<ActorTickDebug3_hook>::unhook();
        ll::memory::HookRegistrar<ActorTickDebug_ActorRemoved_hook>::unhook();
        g_actors.clear();
    }
    return isOn;
}


} // namespace fh
