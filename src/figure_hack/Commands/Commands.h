#pragma once

#include <ll/api/command/Command.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <mc/server/commands/CommandOutput.h>

#include "GiveMagicStick.h"
#include "Info.h"
#include "QueryPendingTick.h"
#include "Tick.h"


namespace fh {

struct Commands {
    static void initAll() {
        InfoCommand::init();
        GiveMagicStickCommand::init();
        TickCommand::init();
        QueryPendingTickCommand::init();
    }
};

} // namespace fh
