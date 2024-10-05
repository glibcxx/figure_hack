#pragma once

#include <ll/api/chrono/GameChrono.h>
#include <ll/api/service/Bedrock.h>
#include <mc/server/commands/CommandPermissionLevel.h>
#include <mc/world/Minecraft.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/dimension/Dimension.h>
#include <mc/world/redstone/circuit/CircuitSystem.h>
#include <queue>

#include "figure_hack/Commands/Commands.h"
#include "figure_hack/Utils/BlockSelector.h"
#include "figure_hack/Utils/Utils.h"


namespace fh {

struct InfoCommand {
    enum class Mode {
        basic = 0,
        circuit,
        source,
    };

    struct Params {
        Mode mode;
    };

    struct Params_2 {
        Mode            mode;
        CommandPosition pos;
    };

    static void init() {
        using ll::i18n_literals::operator""_tr;
        auto& commandHandle = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
            "info",
            "command.info.description"_tr(),
            CommandPermissionLevel::GameDirectors
        );
        commandHandle.overload<Params>().required("mode").execute(
            [](const CommandOrigin& origin, CommandOutput& output, const Params& params) {
                Actor* entity = origin.getEntity();
                if (entity && entity->isPlayer()) {
                    HitResult  result_pos = entity->traceRay(5.2f, false);
                    Dimension* dim        = origin.getDimension();
                    if (result_pos) {
                        InfoCommand::infoAtPos(dim, result_pos.mBlockPos, params.mode, output);
                    } else {
                        output.error("command.info.error.no_block"_tr());
                    }
                } else {
                    output.error("command.info.error.invalid_player"_tr());
                }
            }
        );
        commandHandle.overload<Params_2>().required("mode").required("pos").execute(
            [](const CommandOrigin& origin, CommandOutput& output, const Params_2& params) {
                InfoCommand::infoAtPos(
                    origin.getDimension(),
                    params.pos.getBlockPos(origin.getEntity()->getPosition()),
                    params.mode,
                    output
                );
            }
        );
    }


protected:
    static void infoAtPos(Dimension* dim, const BlockPos& pos, Mode mode, CommandOutput& output) {
        using ll::i18n_literals::operator""_tr;
        if (!dim) {
            return;
        }
        const Block&       block  = dim->getBlockSourceFromMainChunkSource().getBlock(pos);
        CircuitSystem&     system = dim->getCircuitSystem();
        CircuitSceneGraph& graph  = system.mSceneGraph;

        BSelector::add(dim->getDimensionId(), pos, {.color = BSelector::Color::pink});
        switch (mode) {
        default:
        case Mode::basic:
            output.success("{}: {}", pos.toString(), block.getName().getString());
            break;
        case Mode::circuit: {
            BaseCircuitComponent* comp = graph.getBaseComponent(pos);
            if (comp) {
                output.success("command.info.rs_comp_info"_tr(
                    pos.toString(),
                    comp->getStrength(),
                    comp->mSources.mComponents.size(),
                    (std::ptrdiff_t)comp,
                    utils::typeId2Name(comp->getCircuitComponentType())
                ));
            } else {
                output.error("command.info.error.no_circuit_component"_tr(pos.toString()));
            }
            break;
        }
        case Mode::source: {
            BaseCircuitComponent* comp = graph.getBaseComponent(pos);
            if (comp) {
                if (comp->mSources.mComponents.size() == 0) {
                    output.error("command.info.error.no_source"_tr(pos.toString()));
                    break;
                }
                output.success("command.info.sources_of"_tr(pos.toString()));
                size_t num = 1;
                for (auto&& source : comp->mSources.mComponents) {
                    BSelector::add(dim->getDimensionId(), source.mPos, {.color = BSelector::Color::white});
                    output.success("command.info.source_info"_tr(
                        num,
                        utils::typeId2Name(source.mComponent->getCircuitComponentType()),
                        source.mDirectlyPowered,
                        source.mDampening,
                        source.mData,
                        source.mComponent->getStrength(),
                        (std::ptrdiff_t)source.mComponent,
                        source.mPos.toString()
                    ));
                }
            } else {
                output.error("command.info.error.no_circuit_component"_tr(pos.toString()));
            }
            break;
        }
        }
    }
};

} // namespace fh
