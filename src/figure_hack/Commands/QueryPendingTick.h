#pragma once

#include <span>

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
#include <mc/world/level/Level.h>
#include <mc/world/level/chunk/LevelChunk.h>


#include "figure_hack/Utils/BlockSelector.h"

namespace fh {

struct QueryPendingTickCommand {
    struct Params {
        int display_time;
    };

    static void init() {
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
                getPtInfoAtChunkPos(entity->getDimensionBlockSource(), chunkPos, output, 40);
            }
        });

        commandHandle.overload<Params>()
            .required("display_time")
            .execute([](const CommandOrigin& origin, CommandOutput& output, const Params& params) { // NOLINT
                Actor* entity = origin.getEntity();
                if (entity && entity->isPlayer()) {
                    ChunkPos chunkPos{entity->getFeetBlockPos()};
                    getPtInfoAtChunkPos(entity->getDimensionBlockSource(), chunkPos, output, params.display_time);
                }
            });
    }

    static void
    getPtInfoAtChunkPos(BlockSource& region, const ChunkPos& chunkPos, CommandOutput& output, int displayTime) {
        BlockTickingQueue& pt            = region.getChunk(chunkPos)->getTickQueue();
        auto               nextTickQueue = ll::memory::dAccess<std::vector<BlockTickingQueue::BlockTick>>(&pt, 16);
        BlockTickingQueue::TickDataSet copiedQueue;
        copiedQueue.mC = std::move(nextTickQueue);
        if (!copiedQueue.empty()) {
            BlockTickingQueue::TickDataSet activeQueue;
            output.success(
                "[{}] chunk {} has {} pendingTicks ->",
                region.getLevel().getCurrentTick().t,
                chunkPos.toString(),
                copiedQueue.size()
            );
            for (; !copiedQueue.empty();) {
                auto& blockTick = copiedQueue.top();
                if (blockTick.mIsRemoved) {
                    output.success("  {}: removed", blockTick.mData.mPos.toString());
                    BSelector::add(
                        region.getDimensionId(),
                        blockTick.mData.mPos,
                        {.color = BSelector::Color::red, .lifespan = displayTime}
                    );
                } else {
                    nextTickQueue.emplace_back(blockTick);
                }
                (void)copiedQueue.pop();
            }
            copiedQueue.mC = std::move(nextTickQueue);
            uint64_t now   = region.getLevel().getCurrentTick().t;
            for (; !copiedQueue.empty();) {
                auto& blockTick = copiedQueue.top();
                output.success(
                    "  {}: tick time {}, priority {}, {}",
                    blockTick.mData.mPos.toString(),
                    blockTick.mData.mTick.t,
                    blockTick.mData.mPriorityOffset,
                    blockTick.mData.mBlock->getName().getString()
                );
                BSelector::add(
                    region.getDimensionId(),
                    blockTick.mData.mPos,
                    {.color    = blockTick.mData.mTick.t <= now ? BSelector::Color::green : BSelector::Color::yellow,
                     .lifespan = displayTime}
                );
                (void)copiedQueue.pop();
            }
        } else {
            output.error("no pendingTick");
        }
    }
};

} // namespace fh