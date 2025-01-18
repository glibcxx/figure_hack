#include "QueryPendingTick.h"

#include <ll/api/command/Command.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/service/Bedrock.h>

#include <mc/server/commands/CommandOutput.h>
#include <mc/server/commands/CommandPermissionLevel.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/BlockTickingQueue.h>
#include <mc/world/level/ChunkPos.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/chunk/LevelChunk.h>

#include "figure_hack/Utils/BlockHighlight.h"

namespace fh {

using namespace ll::i18n_literals;

struct Params {
    int display_time;
};

void QueryPendingTickCommand::init() {
    using ll::i18n_literals::operator""_tr;
    auto& commandHandle = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        "pdtick",
        "command.pdtick.description"_tr(),
        CommandPermissionLevel::GameDirectors
    );
    commandHandle.overload().execute([](const CommandOrigin& origin, CommandOutput& output) { // NOLINT
        Actor* entity = origin.getEntity();
        if (entity && entity->isPlayer()) {
            ChunkPos chunkPos{entity->getFeetBlockPos()};
            QueryPendingTickCommand::getPtInfoAtChunkPos(entity->getDimensionBlockSource(), chunkPos, output, 40);
        }
    });

    commandHandle.overload<Params>()
        .required("display_time")
        .execute([](const CommandOrigin& origin, CommandOutput& output, const Params& params) { // NOLINT
            Actor* entity = origin.getEntity();
            if (entity && entity->isPlayer()) {
                ChunkPos chunkPos{entity->getFeetBlockPos()};
                QueryPendingTickCommand::getPtInfoAtChunkPos(
                    entity->getDimensionBlockSource(),
                    chunkPos,
                    output,
                    params.display_time
                );
            }
        });
}

void QueryPendingTickCommand::getPtInfoAtChunkPos(
    BlockSource&    region,
    const ChunkPos& chunkPos,
    CommandOutput&  output,
    int             displayTime
) {
    BlockTickingQueue&             pt                  = region.getChunk(chunkPos)->getTickQueue();
    BlockTickingQueue::TickDataSet copiedNextTickQueue = *pt.mNextTickQueue;
    BlockTickingQueue::TickDataSet activeTickQueue;
    if (!copiedNextTickQueue.empty()) {
        output.success(
            "[{}] chunk {} has {} pendingTicks ->",
            region.getLevel().getCurrentTick().tickID,
            chunkPos.toString(),
            copiedNextTickQueue.size()
        );
        for (; !copiedNextTickQueue.empty();) {
            auto& blockTick = copiedNextTickQueue.top();
            if (blockTick.mIsRemoved) {
                output.success("  {}: removed", blockTick.mData.pos->toString());
                BlockHighlightManager::add(
                    region,
                    blockTick.mData.pos,
                    {.color = BlockHighlightManager::Color::red, .lifespan = (uint32_t)displayTime}
                );
            } else {
                activeTickQueue.mC.emplace_back(blockTick);
            }
            (void)copiedNextTickQueue.pop();
        }
        uint64_t now = region.getLevel().getCurrentTick().tickID;
        for (; !activeTickQueue.empty();) {
            auto& blockTick = activeTickQueue.top();
            output.success(
                "  {}: tick time {}, priority {}, {}",
                blockTick.mData.pos->toString(),
                blockTick.mData.tick->tickID,
                blockTick.mData.priorityOffset,
                "{}"_tr(blockTick.mData.mBlock->buildDescriptionName())
            );
            BlockHighlightManager::add(
                region,
                blockTick.mData.pos,
                {.color    = blockTick.mData.tick->tickID <= now ? BlockHighlightManager::Color::green
                                                                 : BlockHighlightManager::Color::yellow,
                 .lifespan = (uint32_t)displayTime}
            );
            (void)activeTickQueue.pop();
        }
    } else {
        output.error("no pendingTick");
    }
}

} // namespace fh