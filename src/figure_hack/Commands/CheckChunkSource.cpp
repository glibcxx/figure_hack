#include "CheckChunkSource.h"

#include <span>

#include <ll/api/command/Command.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/service/Bedrock.h>
#include <mc/server/commands/CommandOutput.h>
#include <mc/server/commands/CommandPermissionLevel.h>


#include "figure_hack/Function/ChunkViewInfo.h"
#include "mc/world/actor/player/Player.h"


namespace fh {

void CheckChunkSourceCommand::init() {
    using ll::i18n_literals::operator""_tr;
    auto& commandHandle = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        "chunksourceinfo",
        "Get ChunkSource Info"_tr(),
        CommandPermissionLevel::GameDirectors
    );
    commandHandle.overload().execute([](const CommandOrigin& origin, CommandOutput& output) { // NOLINT
        Actor* entity = origin.getEntity();
        if (!entity || entity->isPlayer()) {
            queryCurrentChunkSourceInfo(static_cast<Player*>(entity));
        }
    });
}

} // namespace fh
