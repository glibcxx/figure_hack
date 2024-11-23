// #include "fmt/format.h"
// #include "ll/api/service/Bedrock.h"
// #include <ll/api/memory/Hook.h>
// #include <mc/server/ServerPlayer.h>
// #include <mc/world/ActorRuntimeID.h>
// #include <mc/world/actor/item/PrimedTnt.h>
// #include <mc/world/level/BlockSource.h>
// #include <mc/world/level/Level.h>


// namespace fh {

// LL_AUTO_TYPE_INSTANCE_HOOK(
//     DETECT_TNT,
//     ll::memory::HookPriority::Normal,
//     PrimedTnt,
//     "?reloadHardcoded@PrimedTnt@@UEAAXW4ActorInitializationMethod@@AEBVVariantParameterList@@@Z",
//     void,
//     ::ActorInitializationMethod method,
//     VariantParameterList const& params
// ) {
//     ll::service::getLevel()->forEachPlayer([this](Player& player) -> bool {
//         player.sendMessage(fmt::format(
//             "[TNT before][{}] {} ---\n    pos {},\n    speed {},\n    speed length: {},\n",
//             this->getDimensionBlockSource().getLevel().getCurrentTick().t,
//             this->getRuntimeID().id,
//             this->getPosition().toString(),
//             this->getPosDelta().toString(),
//             this->getPosDelta().length()
//         ));
//         return true;
//     });
//     this->origin(method, params);
//     ll::service::getLevel()->forEachPlayer([this](Player& player) -> bool {
//         player.sendMessage(fmt::format(
//             "[TNT after][{}] {} ---\n    pos {},\n    speed {},\n    speed length: {},\n",
//             this->getDimensionBlockSource().getLevel().getCurrentTick().t,
//             this->getRuntimeID().id,
//             this->getPosition().toString(),
//             this->getPosDelta().toString(),
//             this->getPosDelta().length()
//         ));
//         return true;
//     });
// }

// LL_AUTO_TYPE_INSTANCE_HOOK(
//     TICK_TNT,
//     ll::memory::HookPriority::Normal,
//     PrimedTnt,
//     "?normalTick@PrimedTnt@@UEAAXXZ",
//     void
// ) {
//     this->origin();
//     ll::service::getLevel()->forEachPlayer([this](Player& player) -> bool {
//         player.sendMessage(fmt::format(
//             "[TICK TNT][{}] {} ---\n    pos {},\n    speed {},\n    speed length: {},\n",
//             this->getDimensionBlockSource().getLevel().getCurrentTick().t,
//             this->getRuntimeID().id,
//             this->getPosition().toString(),
//             this->getPosDelta().toString(),
//             this->getPosDelta().length()
//         ));
//         return true;
//     });
// }
// } // namespace fh