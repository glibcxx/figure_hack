#pragma once

#include <ll/api/command/Command.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/service/Bedrock.h>
#include <mc/server/commands/CommandOutput.h>
#include <mc/server/commands/CommandPermissionLevel.h>


#include "figure_hack/Function/MagicStick.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/item/enchanting/EnchantUtils.h"


namespace fh {


struct GiveMagicStickCommand {
    struct Params {};

    static void init() {
        using ll::i18n_literals::operator""_tr;
        auto& commandHandle = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
            "mstick",
            "command.mstick.description"_tr(),
            CommandPermissionLevel::GameDirectors
        );
        commandHandle.overload().execute([](const CommandOrigin& origin, CommandOutput& output) { // NOLINT
            Actor* entity = origin.getEntity();
            if (entity && entity->isPlayer()) {
                Player&   player = *static_cast<Player*>(entity);
                ItemStack itemStack{"stick", 1};
                itemStack.setCustomName("item.magic_stick.name"_tr(MagicStick::mode_name[0]));
                EnchantUtils::applyEnchant(itemStack, Enchant::Type::MiningEfficiency, 1, true);
                player.add(itemStack);
                player.refreshInventory();
            }
        });
    }
};

} // namespace fh
