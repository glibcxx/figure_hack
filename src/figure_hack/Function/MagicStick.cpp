#include "MagicStick.h"

#include <ll/api/event/EventBus.h>
#include <ll/api/event/Listener.h>
#include <ll/api/event/player/PlayerAttackEvent.h>
#include <ll/api/event/player/PlayerDestroyBlockEvent.h>
#include <ll/api/event/player/PlayerInteractBlockEvent.h>
#include <ll/api/event/player/PlayerSwingEvent.h>
#include <ll/api/memory/Hook.h>

#include <mc/server/commands/CommandOutput.h>
#include <mc/world/item/Item.h>
#include <mc/world/item/enchanting/EnchantUtils.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/material/Material.h>

#include "CircuitPendingUpdateVisualization.h"
#include "figure_hack/Utils/BlockSelector.h"
#include "figure_hack/Utils/Utils.h"
#include "figure_hack/figure_hack.h"

namespace fh {

namespace {
ll::event::ListenerPtr playerUseMagicStickEventListener;
ll::event::ListenerPtr playerChangeMagicStickModeEventListener;
ll::event::ListenerPtr playerDestroyBlockEventListener;

uint64                       lastUsedTime;
uint64                       lastChangeModeTime;
BlockPos                     lastPos;
std::unique_ptr<BlockLegacy> ob{nullptr};

} // namespace


void MagicStick::enable() {
    using ll::i18n_literals::operator""_tr;

    MagicStick::mode_name = {
        "item.magic_stick.mode.normal"_tr(),
        "item.magic_stick.mode.redstone"_tr(),
        "item.magic_stick.mode.source"_tr(),
        "item.magic_stick.mode.graph"_tr()
    };

    auto& bus = ll::event::EventBus::getInstance();

    // 玩家右键
    playerUseMagicStickEventListener =
        bus.emplaceListener<ll::event::PlayerInteractBlockEvent>([](ll::event::PlayerInteractBlockEvent& event) {
            ServerPlayer&   player = event.self();
            uint64          now    = player.getLevelTimeStamp();
            const BlockPos& pos    = event.blockPos();
            ItemStack&      item   = event.item();

            int enchantLevel = EnchantUtils::getEnchantLevel(Enchant::Type::MiningEfficiency, item);
            if (item.getTypeName() == "minecraft:stick" && enchantLevel != 0) {
                event.cancel();
                if (now - lastUsedTime > 2 || lastPos != pos) {
                    switch (enchantLevel) {
                    case 1:
                        utils::executeCommand(player, fmt::format("info basic {} {} {}", pos.x, pos.y, pos.z));
                        break;
                    case 2:
                        utils::executeCommand(player, fmt::format("info circuit {} {} {}", pos.x, pos.y, pos.z));
                        break;
                    case 3:
                        utils::executeCommand(player, fmt::format("info source {} {} {}", pos.x, pos.y, pos.z));
                        break;
                    case 4:
                        if (figureHack::getInstance().getConfig().function.enable_microtick) {
                            player.sendMessage(fmt::format(
                                "{}: {}",
                                pos.toString(),
                                CPUVisualize::switchPos(player.getDimensionId(), pos)
                            ));
                        } else {
                            player.sendMessage("microtick not enabled");
                        }
                        break;
                    default:
                        break;
                    }
                    lastPos = pos;
                }
                lastUsedTime = now;
            }
        });

    // 玩家左键挥手
    playerChangeMagicStickModeEventListener =
        bus.emplaceListener<ll::event::PlayerSwingEvent>([](ll::event::PlayerSwingEvent& event) {
            ServerPlayer& player    = event.self();
            HitResult     hitResult = player.traceRay(5.5f, false, true);
            const Block&  block     = player.getDimensionBlockSource().getBlock(hitResult.mBlockPos);
            if (block.isInteractiveBlock()) {
                return;
            }
            uint64     now   = player.getLevelTimeStamp();
            ItemStack& item  = const_cast<ItemStack&>(player.getSelectedItem());
            int        level = EnchantUtils::getEnchantLevel(Enchant::Type::MiningEfficiency, item);
            if (now - lastChangeModeTime > 2 && item.getTypeName() == "minecraft:stick" && level != 0) {
                EnchantUtils::removeEnchants(item);
                level = level + 1 > 4 ? 1 : level + 1;
                EnchantUtils::applyEnchant(item, Enchant::Type::MiningEfficiency, level, true);
                item.setCustomName("item.magic_stick.name"_tr(MagicStick::mode_name[level - 1]));
                player.sendMessage("item.magic_stick.changed_to"_tr(MagicStick::mode_name[level - 1]));
                player.refreshInventory();
                CPUVisualize::clearPos();
                BSelector::clear();
            }
            lastChangeModeTime = now;
        });

    // 玩家左键打方块
    playerDestroyBlockEventListener = bus.emplaceListener<ll::event::player::PlayerDestroyBlockEvent>(
        [](ll::event::player::PlayerDestroyBlockEvent& event) {
            ServerPlayer&    player = event.self();
            const ItemStack& item   = player.getSelectedItem();

            if (item.getTypeName() == "minecraft:stick"
                && EnchantUtils::getEnchantLevel(Enchant::Type::MiningEfficiency, item) != 0) {
                event.cancel();
            }
        }
    );
}

void MagicStick::disable() {
    auto& bus = ll::event::EventBus::getInstance();
    bus.removeListener(playerUseMagicStickEventListener);
    bus.removeListener(playerChangeMagicStickModeEventListener);
    bus.removeListener(playerDestroyBlockEventListener);
}

} // namespace fh
