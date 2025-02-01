#include "InfoCommand.h"

#include <ll/api/chrono/GameChrono.h>
#include <ll/api/command/Command.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/service/Bedrock.h>

#include <mc/common/ActorRuntimeID.h>
#include <mc/server/commands/CommandOutput.h>
#include <mc/server/commands/CommandPermissionLevel.h>
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

#include "figure_hack/Function/Info.h"
#include "figure_hack/Utils/BlockHighlight.h"
#include "figure_hack/Utils/Utils.h"
#include "magic_enum.hpp"

namespace {

using ll::i18n_literals::operator""_tr;

bool checkExecuterIsPlayer(Actor* entity, CommandOutput& output) {
    if (!entity || !entity->isPlayer()) { // 必须由玩家执行
        output.error("command.info.error.invalid_player"_tr());
        return false;
    }
    return true;
}

} // namespace

namespace fh {

void InfoCommand::init() {
    using ll::i18n_literals::operator""_tr;
    auto& commandHandle = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        "info",
        "command.info.description"_tr(),
        CommandPermissionLevel::GameDirectors
    );

    commandHandle.overload<ActorInfoParams>().text("actor").optional("mode").execute(
        [](const CommandOrigin& origin, CommandOutput& output, const ActorInfoParams& params) {
            Actor* entity = origin.getEntity();
            if (checkExecuterIsPlayer(entity, output)) {
                BlockSource& region = entity->getDimensionBlockSource();
                bool         isOn   = fh::toggleActorInfo(region, params.mode);
                output.success(
                    "ActorInfo: {}",
                    params.mode == ActorInfoMode::toggle ? (isOn ? "On" : "Off") : magic_enum::enum_name(params.mode)
                );
            }
        }
    );
    commandHandle.overload<Params>().required("mode").optional("pos").execute(
        [](const CommandOrigin& origin, CommandOutput& output, const Params& params) {
            Actor* entity = origin.getEntity();
            if (!checkExecuterIsPlayer(entity, output)) return;

            if (params.pos.mOffset->y == INVALID_POSITION_Y) {
                // 指令参数未指定坐标
                HitResult result = entity->traceRay(5.2f, false);
                if (!result) {
                    output.error("command.info.error.no_block"_tr());
                    return;
                }
                _excute(origin, output, params.mode, result.mBlock);
            } else {
                _excute(origin, output, params.mode, params.pos.getBlockPos(origin.getBlockPosition(), Vec3{0}));
            }
        }
    );
}

void InfoCommand::_excute(const CommandOrigin& origin, CommandOutput& output, Mode mode, const BlockPos& pos) {
    using ll::i18n_literals::operator""_tr;
    Actor*       entity = origin.getEntity();
    BlockSource& region = entity->getDimensionBlockSource();
    switch (mode) {
    case Mode::basic: {
        BlockHighlightManager::add(region, pos, {.color = BlockHighlightManager::Color::pink});
        auto data = fh::blockInfoAtPos(region, pos);
        output.success("{}: {}", pos.toString(), data.name);
        break;
    }
    case Mode::circuit: {
        BlockHighlightManager::add(region, pos, {.color = BlockHighlightManager::Color::pink});
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
        BlockHighlightManager::add(region, pos, {.color = BlockHighlightManager::Color::pink});
        auto data = fh::circuitInfoAtPos(region, pos);
        if (data) {
            if (data->sources.mComponents.empty()) {
                output.error("command.info.error.no_source"_tr(pos.toString()));
            }
            output.success("command.info.sources_of"_tr(pos.toString()));
            size_t num = 1;
            for (auto&& source : data->sources.mComponents) {
                BlockHighlightManager::add(region, source.mPos, {.color = BlockHighlightManager::Color::white});
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
                num++;
            }
        } else {
            output.error("command.info.error.no_circuit_component"_tr(pos.toString()));
        }
        break;
    }
    }
}

} // namespace fh
