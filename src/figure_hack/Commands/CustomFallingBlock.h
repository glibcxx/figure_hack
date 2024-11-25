#pragma once

#include "mc/server/commands/CommandPosition.h"
#include <ll/api/command/Command.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/service/Bedrock.h>

#include <mc/server/commands/CommandBlockName.h>
#include <mc/server/commands/CommandBlockNameResult.h>
#include <mc/server/commands/CommandOutput.h>
#include <mc/server/commands/CommandPermissionLevel.h>

#include <mc/world/Minecraft.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/ActorFactory.h>
#include <mc/world/actor/item/FallingBlock.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/dimension/Dimension.h>

namespace fh {

struct CustomFallingBlockCommand {
    struct Params {
        CommandBlockName blockName;
        CommandPosition  blockPos;
    };

    struct ParamsWithoutPos {
        CommandBlockName blockName;
    };

    static void init() {
        using ll::i18n_literals::operator""_tr;
        auto& commandHandle = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
            "falling",
            "Summon a falling block",
            CommandPermissionLevel::GameDirectors
        );
        commandHandle.overload<Params>()
            .required("blockName")
            .required("blockPos")
            .execute([](const CommandOrigin& origin, CommandOutput& output, const Params& params) {
                BlockSource& region = origin.getDimension()->getBlockSourceFromMainChunkSource();
                BlockPos     pos    = params.blockPos.getBlockPos(origin.getBlockPosition());
                const Block* block  = params.blockName.resolveBlock(0).getBlock();
                if (block) {
                    region.setBlock(pos, *block, 3, nullptr, nullptr);
                    static_cast<const FallingBlock&>(block->getLegacyBlock())
                        .FallingBlock::startFalling(region, pos, *block, false);
                    output.success("falling success");
                } else {
                    output.error("error block");
                }
            });

        commandHandle.overload<ParamsWithoutPos>()
            .required("blockName")
            .execute([](const CommandOrigin& origin, CommandOutput& output, const ParamsWithoutPos& params) {
                BlockSource& region = origin.getDimension()->getBlockSourceFromMainChunkSource();
                BlockPos     pos    = origin.getBlockPosition();
                const Block* block  = params.blockName.resolveBlock(0).getBlock();
                if (block) {
                    region.setBlock(pos, *block, 3, nullptr, nullptr);
                    static_cast<const FallingBlock&>(block->getLegacyBlock())
                        .FallingBlock::startFalling(region, pos, *block, false);
                    output.success("falling success");
                } else {
                    output.error("error block");
                }
            });
    }
};

} // namespace fh