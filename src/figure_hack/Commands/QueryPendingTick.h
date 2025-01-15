#pragma once

#include <ll/api/command/Command.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <mc/server/commands/CommandOutput.h>
#include <mc/world/level/BlockSource.h>

namespace fh {

struct QueryPendingTickCommand {

    struct Params {
        int display_time;
    };

    static void init();

protected:
    static void getPtInfoAtChunkPos(BlockSource& region, const ChunkPos& chunkPos, CommandOutput& output, int displayTime);
};

} // namespace fh