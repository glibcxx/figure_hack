#include "Commands.h"

#include "BlockItemCommand.h"
#include "CustomFallingBlock.h"
#include "GiveMagicStick.h"
#include "InfoCommand.h"
#include "QueryPendingTick.h"
#include "TickCommand.h"

void fh::Commands::initAll() {
    InfoCommand::init();
    GiveMagicStickCommand::init();
    TickCommand::init();
    QueryPendingTickCommand::init();
    CustomFallingBlockCommand::init();
    BlockItemCommand::init();
}