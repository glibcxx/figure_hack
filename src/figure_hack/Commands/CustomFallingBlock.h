#pragma once

#include <mc/server/commands/CommandBlockName.h>
#include <mc/server/commands/CommandBlockNameResult.h>
#include <mc/server/commands/CommandPosition.h>
#include <mc/server/commands/CommandRawText.h>

namespace fh {

struct CustomFallingBlockCommand {
    static constexpr float INVALID_POSITION_Y = std::numeric_limits<float>::min();

    struct Params {
        CommandBlockName blockName;
        CommandPosition  blockPos{
             {0, INVALID_POSITION_Y, 0}
        };
    };

    struct ParamsBlockId {
        int             blockId;
        CommandPosition blockPos{
            {0, INVALID_POSITION_Y, 0}
        };
    };

    struct ParamsNamespacedId {
        CommandRawText  namespaceId;
        CommandPosition blockPos{
            {0, INVALID_POSITION_Y, 0}
        };
    };
    static void init();
};

} // namespace fh