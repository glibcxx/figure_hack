#include "BlockHighlight.h"

#include <ll/api/event/EventBus.h>
#include <ll/api/event/Listener.h>
#include <ll/api/event/server/ServerStoppingEvent.h>
#include <ll/api/memory/Hook.h>
#include <ll/api/service/Bedrock.h>
#include <ll/api/utils/StringUtils.h>
#include <mc/deps/core/math/Vec2.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/deps/core/string/HashedString.h>
#include <mc/deps/ecs/gamerefs_entity/GameRefsEntity.h>
#include <mc/legacy/ActorRuntimeID.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/server/ServerLevel.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/ActorDefinitionIdentifier.h>
#include <mc/world/actor/ActorFactory.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/chunk/LevelChunk.h>
#include <mc/world/phys/AABB.h>
#include <unordered_map>
#include <vector>


namespace fh {

struct RmQueue {
    ActorUniqueID id;
    uint64_t      removeTime;

    bool operator>(const RmQueue& rhs) const { return this->removeTime > rhs.removeTime; }
};

struct PendingAdd {
    BlockSource&                  region;
    ActorDefinitionIdentifier     id;
    BlockPos                      pos;
    BlockHighlightManager::Params params;
};

std::vector<PendingAdd>                                                           addQueue;
std::priority_queue<RmQueue, std::vector<fh::RmQueue>, std::greater<fh::RmQueue>> rmQueue;

std::unordered_map<BlockPos, std::vector<ActorUniqueID>> stableHighlight;

LL_AUTO_TYPE_INSTANCE_HOOK(LoopTick, HookPriority::Normal, ServerLevel, &ServerLevel::$tick, void) {
    for (auto&& [region, id, pos, params] : addQueue) {
        auto   actor      = region.getLevel().getActorFactory().createSpawnedActor(id, nullptr, Vec3{0.5, -0.008, 0.5} + pos, {});
        Actor* highligher = region.getLevel().addEntity(region, actor);
        if (highligher) {
            highligher->setVariant(static_cast<int>(params.color));
            ActorUniqueID uniqueid = highligher->getOrCreateUniqueID();
            if (params.lifespan > 0)
                rmQueue.push({uniqueid, params.lifespan + region.getLevel().getCurrentTick().tickID});
            else stableHighlight[pos].push_back(uniqueid);
        }
    }
    addQueue.clear();
    for (; !rmQueue.empty(); rmQueue.pop()) {
        auto& it = rmQueue.top();
        if (it.removeTime > this->getCurrentTick().tickID) {
            break;
        }
        Actor* actor = this->fetchEntity(it.id, false);
        if (actor) {
            actor->remove();
        }
    }
    this->origin();
}


bool BlockHighlightManager::add(BlockSource& region, const BlockPos& pos, BlockHighlightManager::Params params) {
    ActorDefinitionIdentifier id;
    id.initialize("fh:block_highlight");
    addQueue.emplace_back(region, std::move(id), pos, params);
    return true;
}


bool BlockHighlightManager::addStable(BlockSource& region, const BlockPos& pos, BlockHighlightManager::Color color) {
    ActorDefinitionIdentifier id;
    id.initialize("fh:block_highlight");
    addQueue.emplace_back(region, std::move(id), pos, BlockHighlightManager::Params{color, 0});
    return true;
}

bool BlockHighlightManager::removeStable(BlockSource& region, const BlockPos& pos) {
    if (auto iter = stableHighlight.find(pos); iter != stableHighlight.end()) {
        for (auto&& id : iter->second) {
            Actor* actor = region.getLevel().fetchEntity(id, false);
            if (actor) {
                actor->remove();
            }
        }
        stableHighlight.erase(iter);
        return true;
    }
    return false;
}

} // namespace fh
