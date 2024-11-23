#include <span>
#include "figure_hack.h"

#include <ll/api/Config.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/mod/RegisterHelper.h>
#include <memory>

#include "Commands/Commands.h"
#include "Function/MagicStick.h"
#include "Utils/BlockSelector.h"
#include <span>


namespace fh {

static std::unique_ptr<figureHack> instance;

figureHack& figureHack::getInstance() { return *instance; }

bool figureHack::load() {
    const ll::Logger& logger = this->getSelf().getLogger();
    logger.debug("Loading...");

    const auto& configPath = getSelf().getConfigDir() / "config.json";
    if (!ll::config::loadConfig(mConfig, configPath)) {
        logger.warn("Cannot load configurations from {}", configPath);
        logger.info("Saving default configurations");
        if (!ll::config::saveConfig(mConfig, configPath)) {
            logger.error("Cannot save default configurations to {}", configPath);
            return false;
        }
    }
    ll::i18n::getInstance() =
        std::make_unique<ll::i18n::MultiFileI18N>(ll::i18n::MultiFileI18N(getSelf().getLangDir(), mConfig.i18n_location)
        );

    return true;
}

bool figureHack::enable() {
    const ll::Logger& logger = this->getSelf().getLogger();
    logger.debug("Enabling...");

    Commands::initAll();
    BSelector::init();
    MagicStick::enable();

    return true;
}

bool figureHack::disable() {
    const ll::Logger& logger = this->getSelf().getLogger();
    logger.debug("Disabling...");

    BSelector::clear(true);
    MagicStick::disable();
    ll::config::saveConfig(mConfig, getSelf().getConfigDir() / "config.json");
    return true;
}

} // namespace fh

LL_REGISTER_MOD(fh::figureHack, fh::instance);
