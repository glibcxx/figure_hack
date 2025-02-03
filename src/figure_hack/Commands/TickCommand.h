#pragma once

namespace fh {

struct TickCommand {
    static inline float nowRate  = 20.0f;
    static inline bool  isFreeze = false;

    struct Params {
        float rate;
    };

    struct StepParams {
        int tick = 1;
    };

    static void init();
};

} // namespace fh
