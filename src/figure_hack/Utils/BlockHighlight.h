#pragma once

#include <ll/api/chrono/GameChrono.h>
#include <map>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/dimension/Dimension.h>
#include <unordered_map>


namespace fh {

class BlockHighlightManager {
public:
    enum class Color : int {
        transparent = 0,
        black,
        blue,
        brown,
        cyan,
        gray,
        green,
        light_blue,
        lime,
        magenta,
        pink,
        purple,
        red,
        silver,
        white,
        yellow
    };

    struct Params {
        Color    color    = Color::blue;
        uint32_t lifespan = 20; // 0 means permanent
    };

    static void init() {}

    static bool add(BlockSource& region, const BlockPos& pos, BlockHighlightManager::Params params);

    static bool addStable(BlockSource& region, const BlockPos& pos, BlockHighlightManager::Color color);

    static bool removeStable(BlockSource& region, const BlockPos& pos);
};

} // namespace fh
