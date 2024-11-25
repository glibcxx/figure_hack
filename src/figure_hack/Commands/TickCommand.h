#pragma once

namespace fh {

struct TickCommand {
    static inline float nowRate  = 20.0f;
    static inline bool  isFreeze = false;

    struct Params {
        float rate;
    };

    struct ParamsFreeze {
        enum class FreezeType { freeze = 0, resume } freeze;
    };

    static void init();
};

} // namespace fh
