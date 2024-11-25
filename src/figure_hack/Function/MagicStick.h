#pragma once

#include <span>

#include <array>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/Listener.h>
#include <ll/api/i18n/I18n.h>
#include <mc/world/level/BlockPos.h>


namespace fh {

class MagicStick {
public:
    static void enable();
    static void disable();

    static inline std::array<std::string, 4> mode_name{""};
};

} // namespace fh
