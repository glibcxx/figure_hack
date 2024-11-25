#include "BlockItemCommand.h"

#include <ll/api/command/Command.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/service/Bedrock.h>
#include <mc/server/commands/CommandOutput.h>

#include <mc/world/actor/player/Player.h>
#include <mc/world/item/ItemInstance.h>
#include <mc/world/level/block/Block.h>


namespace fh {

void BlockItemCommand::init() {
    using ll::i18n_literals::operator""_tr;
    auto& commandHandle = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        "blockitem",
        "give you customed block item",
        CommandPermissionLevel::GameDirectors
    );

    commandHandle.overload<Params>()
        .required("blockName")
        .execute([](const CommandOrigin& origin, CommandOutput& output, const Params& params) {
            Actor* entity = origin.getEntity();
            if (!entity || !entity->isPlayer()) {
                output.error("invalid executor");
                return;
            }
            const Block* block = params.blockName.resolveBlock(0).getBlock();
            if (!block) {
                output.error("error block");
                return;
            }
            BlockItemCommand::_additem(output, entity, block);
        });

    commandHandle.overload<ParamsBlockId>().required("blockId").execute(
        [](const CommandOrigin& origin, CommandOutput& output, const ParamsBlockId& params) {
            Actor* entity = origin.getEntity();
            if (!entity || !entity->isPlayer()) {
                output.error("invalid executor");
                return;
            }
            auto block = Block::tryGetFromRegistry(params.blockId, 0);
            if (!block) {
                output.error("error block");
                return;
            }
            BlockItemCommand::_additem(output, entity, block);
        }
    );

    commandHandle.overload<ParamsNamespacedId>()
        .required("namespaceId")
        .execute([](const CommandOrigin& origin, CommandOutput& output, const ParamsNamespacedId& params) {
            Actor* entity = origin.getEntity();
            if (!entity || !entity->isPlayer()) {
                output.error("invalid executor");
                return;
            }
            auto block = Block::tryGetFromRegistry(params.namespaceId.getText());
            if (!block) {
                output.error("error block");
                return;
            }
            BlockItemCommand::_additem(output, entity, block);
        });
}

void BlockItemCommand::_additem(CommandOutput& output, Actor* entity, const Block* block) {
    if (!block) {
        output.error("error block");
        return;
    }
    Player&   player = *static_cast<Player*>(entity);
    ItemStack itemStack{*block, 1};
    player.add(itemStack);
    player.refreshInventory();
    output.success("add item success");
}

} // namespace fh