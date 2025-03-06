#include "TickCommand.h"

#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/service/Bedrock.h>

#include <mc/server/commands/CommandOutput.h>
#include <mc/util/Timer.h>
#include <mc/world/Minecraft.h>

#include "figure_hack/figure_hack.h"

namespace fh {

void Timer__stepTick(Timer& timer, int numSteps) {
    timer.mSteppingTick = numSteps;
    if (timer.mSteppingTick >= 0) {
        float passedSeconds = numSteps / timer.mTicksPerSecond;
        timer.mLastTimestep = passedSeconds;
        timer.mPassedTime   = passedSeconds * timer.mTimeScale;
        timer.mAlpha        = timer.mPassedTime;
    } else {
        timer.mLastMsSysTime = timer.mLastMs = (*timer.mGetTimeMSCallback)();
        timer.mLastTimeSeconds               = timer.mLastMs / 1000.0f;
        timer.mPassedTime                    = 0.0f;
        timer.mLastTimestep                  = 0.0f;
        timer.mFrameStepAlignmentRemainder   = 0.0f;
        timer.mTimeScale                     = 1.0f;
    }
}

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

    commandHandle.overload().text("reset").execute([](const CommandOrigin& origin, CommandOutput& output) { // NOLINT
        auto mc = ll::service::getMinecraft();
        if (mc) {
            mc->setSimTimePause(false);
            mc->setSimTimeScale(1.0f);
            TickCommand::isFreeze = false;
            TickCommand::nowRate  = 20.0f;
            output.success("command.tick.reset.success"_tr());
        }
    });

    commandHandle.overload().text("freeze").execute([](const CommandOrigin& origin, CommandOutput& output) { // NOLINT
        auto mc = ll::service::getMinecraft();
        if (mc) {
            mc->setSimTimePause(true);
            TickCommand::isFreeze = true;
            output.success("command.tick.freeze.freezed"_tr());
        }
    });

    commandHandle.overload<StepParams>().text("step").optional("tick").execute(
        [](const CommandOrigin& origin, CommandOutput& output, const StepParams& params) { // NOLINT
            if (params.tick < 0) {
                output.error("Invalid tick input");
                return;
            }
            auto mc = ll::service::getMinecraft();
            if (mc) {
                Timer__stepTick(mc->mSimTimer, params.tick);
                output.success("Step: {}", params.tick);
            }
        }
    );
}

} // namespace fh
