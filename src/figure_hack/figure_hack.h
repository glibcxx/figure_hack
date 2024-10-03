#pragma once

#include "ll/api/mod/NativeMod.h"

#include "Config.h"

namespace fh {

class figureHack {
    Config mConfig;

public:
    static figureHack& getInstance();

    figureHack(ll::mod::NativeMod& self) : mSelf(self) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    Config getConfig() { return mConfig; }

    /// @return True if the mod is loaded successfully.
    bool load();

    /// @return True if the mod is enabled successfully.
    bool enable();

    /// @return True if the mod is disabled successfully.
    bool disable();

    // TODO: Implement this method if you need to unload the mod.
    // /// @return True if the mod is unloaded successfully.
    // bool unload();

private:
    ll::mod::NativeMod& mSelf;
};

} // namespace fh
