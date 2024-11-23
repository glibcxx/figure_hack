#pragma once

#include <span>
#include <ll/api/chrono/GameChrono.h>
#include <ll/api/command/Command.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/service/Bedrock.h>


#include <mc/server/commands/CommandOutput.h>
#include <mc/server/commands/CommandPermissionLevel.h>
#include <mc/world/ActorRuntimeID.h>
#include <mc/world/Minecraft.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/dimension/Dimension.h>
#include <mc/world/phys/AABB.h>
#include <mc/world/phys/HitResult.h>
#include <mc/world/redstone/circuit/CircuitSystem.h>
#include <queue>


#include "figure_hack/Function/Info.h"
#include "figure_hack/Utils/BlockSelector.h"
#include "figure_hack/Utils/Utils.h"


namespace fh {

struct InfoCommand {
    enum class Mode : uint8_t { basic = 0, circuit, source, actor };

    struct Params {
        Mode mode;
    };

    struct ParamsWithPos {
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
                if (!entity || !entity->isPlayer()) { // 必须由玩家执行
                    output.error("command.info.error.invalid_player"_tr());
                    return;
                }
                if (params.mode == Mode::actor) {
                    _excute(origin, output, params.mode, {});
                } else {
                    HitResult result = entity->traceRay(5.2f, false);
                    if (!result) {
                        output.error("command.info.error.no_block"_tr());
                        return;
                    }
                    _excute(origin, output, params.mode, result.mBlockPos);
                }
            }
        );
        commandHandle.overload<ParamsWithPos>().required("mode").required("pos").execute(
            [](const CommandOrigin& origin, CommandOutput& output, const ParamsWithPos& params) {
                Actor* entity = origin.getEntity();
                if (!entity || !entity->isPlayer()) { // 必须由玩家执行
                    output.error("command.info.error.invalid_player"_tr());
                    return;
                }
                _excute(origin, output, params.mode, params.pos.getBlockPos(entity->getPosition()));
            }
        );
    }


protected:
    static void _excute(const CommandOrigin& origin, CommandOutput& output, Mode mode, const BlockPos& pos) {
        using ll::i18n_literals::operator""_tr;
        Actor* entity = origin.getEntity();
        if (!entity || !entity->isPlayer()) { // 必须由玩家执行
            output.error("command.info.error.invalid_player"_tr());
            return;
        }
        BlockSource& region = entity->getDimensionBlockSource();
        switch (mode) {
        case Mode::actor: {
            auto data =
                fh::actorInfo(entity, region, entity->getEyePos(), entity->getEyePos() + entity->getViewVector() * 5.2f);
            if (data) {
                output.success(
                    "[{}] {} id {} ---\n  pos ({:.16f}, {:.16f}, {:.16f}),\n  vel |({:.16f}, {:.16f}, {:.16f})| =  "
                    "{:.16f}",
                    origin.getLevel()->getCurrentTick().t,
                    data->typeId,
                    data->runtimeId.id,
                    data->pos.x,
                    data->pos.y,
                    data->pos.z,
                    data->velocity.x,
                    data->velocity.y,
                    data->velocity.z,
                    data->velocity.length()
                );
            } else {
                output.error("command.info.error.no_actor_found"_tr());
            }
            break;
        }
        case Mode::basic: {
            auto data = fh::blockInfoAtPos(region, pos);
            output.success("{}: {}", pos.toString(), data.name);
            break;
        }
        case Mode::circuit: {
            auto data = fh::circuitInfoAtPos(region, pos);
            if (data) {
                output.success("command.info.rs_comp_info"_tr(
                    pos.toString(),
                    data->strength,
                    data->sources.mComponents.size(),
                    (std::ptrdiff_t)data->rawPtr,
                    utils::typeId2Name(data->typeId)
                ));
            } else {
                output.error("command.info.error.no_circuit_component"_tr(pos.toString()));
            }
            break;
        }
        case Mode::source: {
            auto data = fh::circuitInfoAtPos(region, pos);
            if (data) {
                if (data->sources.mComponents.empty()) {
                    output.error("command.info.error.no_source"_tr(pos.toString()));
                }
                size_t num = 1;
                for (auto&& source : data->sources.mComponents) {
                    BSelector::add(entity->getDimensionId(), source.mPos, {.color = BSelector::Color::white});
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
