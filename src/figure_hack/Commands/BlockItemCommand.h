#pragma once

#include <mc/server/commands/CommandBlockName.h>
#include <mc/server/commands/CommandBlockNameResult.h>
#include <mc/server/commands/CommandPosition.h>
#include <mc/server/commands/CommandRawText.h>

class Actor;

namespace fh {

struct BlockItemCommand {
    struct Params {
        CommandBlockName blockName;
    };

    struct ParamsBlockId {
        int blockId;
    };

    struct ParamsNamespacedId {
        CommandRawText namespaceId;
    };

    static void init();

protected:
    static void _additem(CommandOutput& output, Actor* entity, const Block* block);
};

} // namespace fh