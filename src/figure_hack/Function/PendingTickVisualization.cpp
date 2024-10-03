#include <fmt/format.h>
#include <ll/api/memory/Hook.h>
#include <ll/api/memory/Memory.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/BlockTickingQueue.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/block/Block.h>


namespace fh {

// LL_AUTO_TYPE_INSTANCE_HOOK(
//     PT_Hook,
//     ll::memory::HookPriority::Normal,
//     BlockTickingQueue,
//     &BlockTickingQueue::add,
//     void,
//     BlockSource&    region,
//     const BlockPos& pos,
//     const Block&    block,
//     int             tickDelay,
//     int             priorityOffset
// ) {
//     // std::cout << fmt::format(
//     //     "[TP-add][{}] >> {}: {}, {}, {}\n",
//     //     region.getLevel().getCurrentTick().t,
//     //     pos.toString(),
//     //     block.getName().str,
//     //     tickDelay,
//     //     priorityOffset
//     // );

//     this->origin(region, pos, block, tickDelay, priorityOffset);
// }

// LL_AUTO_TYPE_INSTANCE_HOOK(
//     tickPT_Hook,
//     ll::memory::HookPriority::Normal,
//     BlockTickingQueue,
//     &BlockTickingQueue::tickPendingTicks,
//     bool,
//     BlockSource& region,
//     const Tick&  until,
//     int          max,
//     bool         instaTick_
// ) {
//     // std::cout << fmt::format(
//     //     "[TP-add][{}] >> {}: {}, {}, {}\n",
//     //     region.getLevel().getCurrentTick().t,
//     //     pos.toString(),
//     //     block.getName().str,
//     //     tickDelay,
//     //     priorityOffset
//     // );

//     auto mNextTickQueue = ll::memory::dAccess<std::vector<BlockTickingQueue::BlockTick>>(this, 16);
//     if (mNextTickQueue.size()) {
//         std::cout << fmt::format("[TP-tick][{}] ({})\n", region.getLevel().getCurrentTick().t, mNextTickQueue.size());
//     }
//     for (auto&& it : mNextTickQueue) {
//         std::cout << fmt::format(
//             "    >> {}, {}, {}, {}\n",
//             it.mData.mPos.toString(),
//             it.mData.mBlock->getName().str,
//             it.mData.mTick.t,
//             it.mData.mPriorityOffset
//         );
//     }


//     return this->origin(region, until, max, instaTick_);
// }

} // namespace fh