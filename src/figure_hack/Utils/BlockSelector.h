#pragma once

#include <ll/api/chrono/GameChrono.h>
#include <map>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/dimension/Dimension.h>
#include <unordered_map>

namespace fh {

struct LoopTick;

class BSelector {
public:
    enum class Color {
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

    struct BlockProperty {
        Color color    = Color::blue;
        int   lifespan = 20;
    };

protected:
    struct Internal {
        DimensionType                                          dimensionType;
        std::unordered_map<BlockPos, std::stack<const Block*>> exBlockStorage;

        std::multimap<ll::chrono::GameTickClock::time_point, BlockPos> removeQueue;
        std::multimap<BlockPos, BlockProperty>                         addQueue;

        Internal() = default;

        Internal(DimensionType id) : dimensionType(id) {}

        bool add(const BlockPos& pos, BlockProperty property);

        bool removeStable(const BlockPos& pos);

        void removeAll(bool immediately);

        void tick();

    protected:
        void processAdd();

        void processRemove();

        void removeByPos(const BlockPos& pos);
    };


    static inline std::unordered_map<DimensionType, Internal> selector{};

    friend struct LoopTick;

public:
    static void init();

    static bool add(DimensionType dimType, const BlockPos& pos, BSelector::BlockProperty property);

    static bool addStable(DimensionType dimType, const BlockPos& pos, BSelector::Color color = Color::blue);

    static void removeStable(DimensionType dimType, const BlockPos& pos);

    static void clear(bool immediately = false);

    static const std::array<const Block*, 16>& getSelectorBlocks();

    static bool isSelectorBlock(const Block& other);
};

} // namespace fh
