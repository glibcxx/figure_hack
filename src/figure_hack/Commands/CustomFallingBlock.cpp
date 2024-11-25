#include "CustomFallingBlock.h"

#include <span>

#include <cstdint>
#include <limits>
#include <ll/api/command/Command.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/service/Bedrock.h>


#include <mc/server/commands/CommandOutput.h>
#include <mc/server/commands/CommandPermissionLevel.h>


#include <mc/world/Minecraft.h>
#include <mc/world/actor/item/FallingBlock.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/dimension/Dimension.h>

namespace fh {

void _spwanFallingBlock(CommandOutput& output, BlockSource& region, const BlockPos& pos, const Block* block) {
    if (block) {
        region.setBlock(pos, *block, 3, nullptr, nullptr);
        static_cast<const FallingBlock&>(block->getLegacyBlock())
            .FallingBlock::startFalling(region, pos, *block, false);
        output.success("falling success");
    } else {
        output.error("error block");
    }
}


void CustomFallingBlockCommand::init() {
    using ll::i18n_literals::operator""_tr;
    auto& commandHandle = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        "falling",
        "Summon a falling block",
        CommandPermissionLevel::GameDirectors
    );
    commandHandle.overload<Params>()
        .required("blockName")
        .optional("blockPos")
        .execute([](const CommandOrigin& origin, CommandOutput& output, const Params& params) {
            _spwanFallingBlock(
                output,
                origin.getDimension()->getBlockSourceFromMainChunkSource(),
                params.blockPos.mOffset.y != INVALID_POSITION_Y ? params.blockPos.getBlockPos(origin.getBlockPosition())
                                                                : origin.getBlockPosition(),
                params.blockName.resolveBlock(0).getBlock()
            );
        });

    commandHandle.overload<ParamsBlockId>()
        .required("blockId")
        .optional("blockPos")
        .execute([](const CommandOrigin& origin, CommandOutput& output, const ParamsBlockId& params) {
            _spwanFallingBlock(
                output,
                origin.getDimension()->getBlockSourceFromMainChunkSource(),
                params.blockPos.mOffset.y != INVALID_POSITION_Y ? params.blockPos.getBlockPos(origin.getBlockPosition())
                                                                : origin.getBlockPosition(),
                Block::tryGetFromRegistry(params.blockId, 0).as_ptr()
            );
        });

    commandHandle.overload<ParamsNamespacedId>()
        .required("namespaceId")
        .optional("blockPos")
        .execute([](const CommandOrigin& origin, CommandOutput& output, const ParamsNamespacedId& params) {
            _spwanFallingBlock(
                output,
                origin.getDimension()->getBlockSourceFromMainChunkSource(),
                params.blockPos.mOffset.y != INVALID_POSITION_Y ? params.blockPos.getBlockPos(origin.getBlockPosition())
                                                                : origin.getBlockPosition(),
                Block::tryGetFromRegistry(params.namespaceId.getText()).as_ptr()
            );
        });
}


} // namespace fh