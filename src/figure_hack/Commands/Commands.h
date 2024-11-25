#pragma once

#include "CustomFallingBlock.h"
#include "GiveMagicStick.h"
#include "InfoCommand.h"
#include "QueryPendingTick.h"
#include "TickCommand.h"

namespace fh {

struct Commands {
    static void initAll() {
        InfoCommand::init();
        GiveMagicStickCommand::init();
        TickCommand::init();
        QueryPendingTickCommand::init();
        CustomFallingBlockCommand::init();
    }
};

} // namespace fh
