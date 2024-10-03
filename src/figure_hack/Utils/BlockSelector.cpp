#include "BlockSelector.h"

#include <ll/api/event/EventBus.h>
#include <ll/api/event/Listener.h>
#include <ll/api/event/player/PlayerLeaveEvent.h>
#include <ll/api/event/server/ServerStoppingEvent.h>
#include <ll/api/memory/Hook.h>
#include <ll/api/service/Bedrock.h>
#include <ll/api/utils/StringUtils.h>
#include <mc/server/ServerLevel.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/chunk/LevelChunk.h>
#include <unordered_map>

namespace fh {

std::mutex             uniMutex;
ll::event::ListenerPtr playerDisconnect;

LL_AUTO_TYPE_INSTANCE_HOOK(LoopTick, HookPriority::Normal, ServerLevel, &ServerLevel::tick, void) {
    for (auto&& it : BSelector::selector) {
        it.second.tick();
    }
    this->origin();
}

LL_AUTO_TYPE_INSTANCE_HOOK( // NOLINT
    LC_placeCallbacks_Hook,
    ll::memory::HookPriority::Normal,
    LevelChunk,
    &LevelChunk::placeCallbacks,
    void,
    const ChunkBlockPos&        pos,
    const Block&                old,
    const Block&                current,
    BlockSource*                currentSource,
    std::shared_ptr<BlockActor> blockEntity
) {
    if (BSelector::isSelectorBlock(old) || BSelector::isSelectorBlock(current)) {
        return;
    }
    this->origin(pos, old, current, currentSource, blockEntity);
}

} // namespace fh

bool fh::BSelector::Internal::add(const BlockPos& pos, BlockProperty property) {
    std::unique_lock lock{uniMutex};
    addQueue.emplace(pos, property);
    return true;
}

bool fh::BSelector::Internal::removeStable(const BlockPos& pos) {
    std::unique_lock lock{uniMutex};
    auto             found = exBlockStorage.find(pos);
    if (found != exBlockStorage.end()) {
        this->removeByPos(pos);
        return true;
    }
    return false;
}

void fh::BSelector::Internal::removeAll(bool immediately) {
    auto                  now = ll::chrono::GameTickClock::now();
    std::vector<BlockPos> tmp;
    tmp.reserve(this->removeQueue.size());
    std::unique_lock lock{uniMutex};
    for (auto&& [time, pos] : this->removeQueue) {
        tmp.emplace_back(pos);
    }
    this->removeQueue.clear();
    for (auto&& pos : tmp) {
        this->removeQueue.emplace(now, pos);
    }
    if (immediately) {
        this->processRemove();
    }
}

void fh::BSelector::Internal::tick() {
    using namespace std::chrono_literals;
    std::unique_lock lock{uniMutex};
    this->processRemove();
    this->processAdd();
}

void fh::BSelector::Internal::processAdd() {
    for (auto&& [pos, property] : this->addQueue) {
        auto level = ll::service::bedrock::getLevel();
        if (level) {
            auto           dim        = level->getDimension(this->dimensionType);
            ::BlockSource& region     = dim->getBlockSourceFromMainChunkSource();
            auto&          blockToSet = BSelector::getSelectorBlocks()[static_cast<int>(property.color)];
            if (region.getBlock(pos).isAir()) {
                region.setBlock(pos, *blockToSet, 2, nullptr, nullptr);
            } else {
                exBlockStorage[pos].emplace(&region.getExtraBlock(pos));
                region.setExtraBlock(pos, *blockToSet, 2);
            }
            if (property.lifespan >= 0) {
                this->removeQueue.emplace(ll::chrono::GameTickClock::now() + ll::chrono::ticks{property.lifespan}, pos);
            }
        }
    }
    this->addQueue.clear();
}

void fh::BSelector::Internal::processRemove() {
    for (auto iter = this->removeQueue.begin(); !this->removeQueue.empty();) {
        auto&& [timeToRemove, pos] = *iter;
        if (timeToRemove <= ll::chrono::GameTickClock::now()) {
            this->removeByPos(pos);
            iter = this->removeQueue.erase(iter);
        } else {
            break;
        }
    }
}

void fh::BSelector::Internal::removeByPos(const BlockPos& pos) {
    auto& stack = this->exBlockStorage[pos];
    auto  level = ll::service::bedrock::getLevel();
    if (level) {
        auto         dim    = level->getDimension(this->dimensionType);
        BlockSource& region = dim->getBlockSourceFromMainChunkSource();
        if (stack.empty()) {
            region.setBlock(pos, region.getEmptyBlock(), 2, nullptr, nullptr);
        } else {
            region.setExtraBlock(pos, *stack.top(), 2);
            stack.pop();
        }
    }
    if (stack.empty()) {
        this->exBlockStorage.erase(pos);
    }
}

void fh::BSelector::init() {
    auto& bus        = ll::event::EventBus::getInstance();
    playerDisconnect = bus.emplaceListener<ll::event::ServerStoppingEvent>([](ll::event::ServerStoppingEvent&) {
        BSelector::clear(true);
    });
}

bool fh::BSelector::add(DimensionType dimType, const BlockPos& pos, BlockProperty property = {}) {
    auto found = fh::BSelector::selector.find(dimType);
    if (found == fh::BSelector::selector.end()) {
        fh::BSelector::selector.emplace(dimType, dimType);
    }
    return fh::BSelector::selector[dimType].add(pos, property);
}

bool fh::BSelector::addStable(DimensionType dimType, const BlockPos& pos, Color color) {
    auto found = fh::BSelector::selector.find(dimType);
    if (found == fh::BSelector::selector.end()) {
        fh::BSelector::selector.emplace(dimType, dimType);
    }
    return fh::BSelector::selector[dimType].add(pos, {color, -1});
}

void fh::BSelector::removeStable(DimensionType dimType, const BlockPos& pos) {
    fh::BSelector::selector[dimType].removeStable(pos);
}

void fh::BSelector::clear(bool immediately) {
    for (auto&& [type, internal] : BSelector::selector) {
        internal.removeAll(immediately);
    }
}

const std::array<const Block*, 16>& fh::BSelector::getSelectorBlocks() {
    static std::array<const Block*, 16> blocks = {
        Block::tryGetFromRegistry("fh:block_selector"),
        Block::tryGetFromRegistry("fh:block_selector_black"),
        Block::tryGetFromRegistry("fh:block_selector_blue"),
        Block::tryGetFromRegistry("fh:block_selector_brown"),
        Block::tryGetFromRegistry("fh:block_selector_cyan"),
        Block::tryGetFromRegistry("fh:block_selector_gray"),
        Block::tryGetFromRegistry("fh:block_selector_green"),
        Block::tryGetFromRegistry("fh:block_selector_light_blue"),
        Block::tryGetFromRegistry("fh:block_selector_lime"),
        Block::tryGetFromRegistry("fh:block_selector_magenta"),
        Block::tryGetFromRegistry("fh:block_selector_pink"),
        Block::tryGetFromRegistry("fh:block_selector_purple"),
        Block::tryGetFromRegistry("fh:block_selector_red"),
        Block::tryGetFromRegistry("fh:block_selector_silver"),
        Block::tryGetFromRegistry("fh:block_selector_white"),
        Block::tryGetFromRegistry("fh:block_selector_yellow"),
    };
    return blocks;
}

bool fh::BSelector::isSelectorBlock(const Block& other) {
    static std::unordered_set<HashedString> hashedStringSet = {
        Block::tryGetFromRegistry("fh:block_selector")->getLegacyBlock().getRawNameHash(),
        Block::tryGetFromRegistry("fh:block_selector_black")->getLegacyBlock().getRawNameHash(),
        Block::tryGetFromRegistry("fh:block_selector_blue")->getLegacyBlock().getRawNameHash(),
        Block::tryGetFromRegistry("fh:block_selector_brown")->getLegacyBlock().getRawNameHash(),
        Block::tryGetFromRegistry("fh:block_selector_cyan")->getLegacyBlock().getRawNameHash(),
        Block::tryGetFromRegistry("fh:block_selector_gray")->getLegacyBlock().getRawNameHash(),
        Block::tryGetFromRegistry("fh:block_selector_green")->getLegacyBlock().getRawNameHash(),
        Block::tryGetFromRegistry("fh:block_selector_light_blue")->getLegacyBlock().getRawNameHash(),
        Block::tryGetFromRegistry("fh:block_selector_lime")->getLegacyBlock().getRawNameHash(),
        Block::tryGetFromRegistry("fh:block_selector_magenta")->getLegacyBlock().getRawNameHash(),
        Block::tryGetFromRegistry("fh:block_selector_pink")->getLegacyBlock().getRawNameHash(),
        Block::tryGetFromRegistry("fh:block_selector_purple")->getLegacyBlock().getRawNameHash(),
        Block::tryGetFromRegistry("fh:block_selector_red")->getLegacyBlock().getRawNameHash(),
        Block::tryGetFromRegistry("fh:block_selector_silver")->getLegacyBlock().getRawNameHash(),
        Block::tryGetFromRegistry("fh:block_selector_white")->getLegacyBlock().getRawNameHash(),
        Block::tryGetFromRegistry("fh:block_selector_yellow")->getLegacyBlock().getRawNameHash()
    };
    return hashedStringSet.contains(other.getLegacyBlock().getRawNameHash());
}
