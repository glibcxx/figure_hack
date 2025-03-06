#include "figure_hack.h"

#include <ll/api/Config.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/mod/RegisterHelper.h>

#include "Commands/Commands.h"
#include "Function/MagicStick.h"
#include "Utils/BlockHighlight.h"

namespace fh {

figureHack& figureHack::getInstance() {
    static figureHack instance;
    return instance;
}

bool figureHack::load() {
    const ll::io::Logger& logger = this->getSelf().getLogger();
    logger.debug("Loading...");

    const auto& configPath = this->getSelf().getConfigDir() / "config.json";
    if (!ll::config::loadConfig(mConfig, configPath)) {
        logger.warn("Cannot load configurations from {}", configPath);
        logger.info("Saving default configurations");
        if (!ll::config::saveConfig(mConfig, configPath)) {
            logger.error("Cannot save default configurations to {}", configPath);
            return false;
        }
    }
    auto res = ll::i18n::getInstance().load(this->getSelf().getLangDir());
    return true;
}

bool figureHack::enable() {
    const ll::io::Logger& logger = this->getSelf().getLogger();
    logger.debug("Enabling...");

    Commands::initAll();
    BlockHighlightManager::init();
    MagicStick::enable();

    return true;
}

bool figureHack::disable() {
    const ll::io::Logger& logger = this->getSelf().getLogger();
    logger.debug("Disabling...");

    MagicStick::disable();
    ll::config::saveConfig(mConfig, getSelf().getConfigDir() / "config.json");
    return true;
}

} // namespace fh

LL_REGISTER_MOD(fh::figureHack, fh::figureHack::getInstance());
