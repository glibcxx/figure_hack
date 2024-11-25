#pragma once

#include <mc/server/commands/CommandPosition.h>

namespace fh {

struct InfoCommand {

    enum class Mode : uint8_t { basic = 0, circuit, source, actor };

    static constexpr float INVALID_POSITION_Y = std::numeric_limits<float>::min();

    struct Params {
        Mode            mode;
        CommandPosition pos{
            {0, INVALID_POSITION_Y, 0}
        };
    };

    static void init();

protected:
    static void _excute(const CommandOrigin& origin, CommandOutput& output, Mode mode, const BlockPos& pos);
};

} // namespace fh
