#include "ChunkViewInfo.h"

#include <memory>
#include <mutex>
#include <span>

#include "ll/api/Logger.h"
#include "ll/api/memory/Hook.h"
#include <ll/api/memory/Hook.h>
#include <ll/api/schedule/Scheduler.h>
#include <ll/api/service/Bedrock.h>
#include <ll/api/thread/ThreadPool.h>

#include <mc/world/actor/player/Player.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/chunk/ChunkSource.h>
#include <mc/world/level/chunk/ChunkViewSource.h>
#include <mc/world/level/dimension/Dimension.h>
#include <unordered_map>


namespace {

std::unordered_map<
    ::Bedrock::EnableNonOwnerReferences*,
    std::weak_ptr<::Bedrock::EnableNonOwnerReferences::ControlBlock>>
    cappturedChunkSources;

Vec3 getBoundsCenter(const Bounds& b) { return Vec3(b.min.x + b.max.x, b.min.y + b.max.y, b.min.z + b.max.z) * 0.5f; }

} // namespace


namespace fh {

void sendToPlayerOrConsole(Player* player, std::string_view msg) {
    if (player && player->isPlayerInitialized()) {
        player->sendMessage(msg);
    } else {
        std::cout << msg << '\n';
    }
}

void queryCurrentChunkSourceInfo(Player* player) {
    if (cappturedChunkSources.empty()) {
        sendToPlayerOrConsole(player, "[ChunkSource] no ChunkSource\n");
        return;
    }
    sendToPlayerOrConsole(player, fmt::format("[Info] {} captured ChunkSources ->\n", cappturedChunkSources.size()));
    size_t removed_count = 0;
    for (auto it = cappturedChunkSources.begin(); it != cappturedChunkSources.end();) {
        auto sptr = it->second.lock();
        if (sptr) {
            ChunkViewSource* chunkSource = static_cast<ChunkViewSource*>(sptr->mPtr);
            sendToPlayerOrConsole(
                player,
                fmt::format(
                    "  center: {}, size: {}, circle: {}, dimId: {} {:X}\n",
                    getBoundsCenter(chunkSource->mArea.mBounds).toString(),
                    chunkSource->mArea.mBounds.mDim.toString(),
                    chunkSource->mArea.mCircle,
                    chunkSource->getDimension().getDimensionId().id,
                    (uintptr_t)chunkSource
                )
            );
            ++it;
        } else {
            ++removed_count;
            it = cappturedChunkSources.erase(it);
        }
        if (removed_count > 0) sendToPlayerOrConsole(player, fmt::format("  {} ChunkSources removed", removed_count));
    }
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    ChunkSourceInfo_hook,
    ll::memory::HookPriority::Normal,
    ChunkViewSource,
    "?move@ChunkViewSource@@QEAAXAEBUBounds@@_NW4ChunkSourceViewGenerateMode@@V?$function@$$A6AXV?$buffer_span_mut@V?$"
    "shared_ptr@VLevelChunk@@@std@@@@V?$buffer_span@I@@@Z@std@@PEBM@Z",
    void,
    struct Bounds const&                                                                                   bounds,
    bool                                                                                                   isCircle,
    ::ChunkSourceViewGenerateMode                                                                          mode,
    std::function<void(class buffer_span_mut<std::shared_ptr<class LevelChunk>>, class buffer_span<uint>)> add,
    float const*                                                                                           f
) {
    this->origin(bounds, isCircle, mode, add, f);
    cappturedChunkSources.emplace(this->mControlBlock->mPtr, std::weak_ptr{this->mControlBlock});
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    ChunkSourceInfo_hook2,
    ll::memory::HookPriority::Normal,
    ChunkViewSource,
    "?move@ChunkViewSource@@QEAAXAEBVBlockPos@@H_NW4ChunkSourceViewGenerateMode@@V?$function@$$A6AXV?$buffer_span_mut@"
    "V?$shared_ptr@VLevelChunk@@@std@@@@V?$buffer_span@I@@@Z@std@@PEBM@Z",
    void,
    class BlockPos const&                                                                                  center,
    int                                                                                                    radius,
    bool                                                                                                   isCircle,
    ::ChunkSourceViewGenerateMode                                                                          mode,
    std::function<void(class buffer_span_mut<std::shared_ptr<class LevelChunk>>, class buffer_span<uint>)> add,
    float const*                                                                                           f
) {
    this->origin(center, radius, isCircle, mode, add, f);
    cappturedChunkSources.emplace(this->mControlBlock->mPtr, std::weak_ptr{this->mControlBlock});
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    ChunkSourceInfo_hook3,
    ll::memory::HookPriority::Normal,
    ChunkViewSource,
    "?move@ChunkViewSource@@QEAAXAEBVBlockPos@@0_NW4ChunkSourceViewGenerateMode@@V?$function@$$A6AXV?$buffer_span_mut@"
    "V?$shared_ptr@VLevelChunk@@@std@@@@V?$buffer_span@I@@@Z@std@@PEBM@Z",
    void,
    class BlockPos const&                                                                                  blockMin,
    class BlockPos const&                                                                                  blockMax,
    bool                                                                                                   isCircle,
    ::ChunkSourceViewGenerateMode                                                                          mode,
    std::function<void(class buffer_span_mut<std::shared_ptr<class LevelChunk>>, class buffer_span<uint>)> add,
    float const*                                                                                           f
) {
    this->origin(blockMin, blockMax, isCircle, mode, add, f);
    cappturedChunkSources.emplace(this->mControlBlock->mPtr, std::weak_ptr{this->mControlBlock});
}

} // namespace fh
