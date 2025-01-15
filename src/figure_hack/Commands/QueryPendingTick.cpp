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

#include "figure_hack/Utils/BlockSelector.h"

namespace fh {

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
    BlockTickingQueue&             pt            = region.getChunk(chunkPos)->getTickQueue();
    auto                           nextTickQueue = pt.mNextTickQueue;
    BlockTickingQueue::TickDataSet copiedQueue   = nextTickQueue;
    if (!copiedQueue.empty()) {
        BlockTickingQueue::TickDataSet activeQueue;
        output.success(
            "[{}] chunk {} has {} pendingTicks ->",
            region.getLevel().getCurrentTick().tickID,
            chunkPos.toString(),
            copiedQueue.size()
        );
        for (; !copiedQueue.empty();) {
            auto& blockTick = copiedQueue.top();
            if (blockTick.mIsRemoved) {
                output.success("  {}: removed", blockTick.mData.pos->toString());
                BSelector::add(
                    region.getDimensionId(),
                    blockTick.mData.pos,
                    {.color = BSelector::Color::red, .lifespan = displayTime}
                );
            } else {
                nextTickQueue->mC.emplace_back(blockTick);
            }
            (void)copiedQueue.pop();
        }
        copiedQueue  = std::move(nextTickQueue);
        uint64_t now = region.getLevel().getCurrentTick().tickID;
        for (; !copiedQueue.empty();) {
            auto& blockTick = copiedQueue.top();
            output.success(
                "  {}: tick time {}, priority {}, {}",
                blockTick.mData.pos->toString(),
                blockTick.mData.tick->tickID,
                blockTick.mData.priorityOffset,
                blockTick.mData.mBlock->buildDescriptionName()
            );
            BSelector::add(
                region.getDimensionId(),
                blockTick.mData.pos,
                {.color    = blockTick.mData.tick->tickID <= now ? BSelector::Color::green : BSelector::Color::yellow,
                 .lifespan = displayTime}
            );
            (void)copiedQueue.pop();
        }
    } else {
        output.error("no pendingTick");
    }
}

} // namespace fh