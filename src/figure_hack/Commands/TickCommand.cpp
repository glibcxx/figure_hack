#include "TickCommand.h"

#include <span>

#include <ll/api/command/Command.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/service/Bedrock.h>

#include <mc/server/commands/CommandOutput.h>
#include <mc/server/commands/CommandPermissionLevel.h>

#include "figure_hack/figure_hack.h"
#include "mc/world/Minecraft.h"

namespace fh {


void TickCommand::init() {
    if (!figureHack::getInstance().getConfig().function.tick_command) return;

    using ll::i18n_literals::operator""_tr;
    auto& commandHandle = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        "tick",
        "command.tick.description"_tr(),
        CommandPermissionLevel::GameDirectors
    );
    commandHandle.overload<Params>().required("rate").execute(
        [](const CommandOrigin& origin, CommandOutput& output, const Params& params) { // NOLINT
            if (isFreeze) {
                output.error("command.tick.rate.freeze_now"_tr());
                return;
            }
            if (params.rate <= 0) {
                output.error("command.tick.rate.invalid_rate"_tr());
                return;
            }
            auto mc = ll::service::getMinecraft();
            if (mc) {
                mc->setSimTimeScale(1.0f);
                mc->setSimTimeScale(params.rate / 20.0f);
                TickCommand::nowRate = params.rate;
                output.success("command.tick.rate.success"_tr(params.rate));
            }
        }
    );

    commandHandle.overload().text("reset").execute([](const CommandOrigin& origin, // NOLINT
                                                      CommandOutput&       output) {
        if (TickCommand::isFreeze) {
            output.error("command.tick.rate.freeze_now"_tr());
            return;
        }
        auto mc = ll::service::getMinecraft();
        if (mc) {

            mc->setSimTimeScale(1.0f);
            TickCommand::nowRate = 20.0f;
            output.success("command.tick.reset.success"_tr());
        }
    });

    commandHandle.overload<ParamsFreeze>().required("freeze").execute(
        [](const CommandOrigin& origin, CommandOutput& output, const ParamsFreeze& params) { // NOLINT
            auto mc = ll::service::getMinecraft();
            if (mc) {
                if (params.freeze == ParamsFreeze::FreezeType::freeze) {
                    mc->setSimTimePause(true);
                    TickCommand::isFreeze = true;
                    output.success("command.tick.freeze.freezed"_tr());
                } else {
                    mc->setSimTimePause(false);
                    mc->setSimTimeScale(TickCommand::nowRate / 20.0f);
                    TickCommand::isFreeze = false;
                    output.success("command.tick.freeze.resumed"_tr());
                }
            }
        }
    );
}

} // namespace fh
