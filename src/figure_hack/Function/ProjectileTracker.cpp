// #include "fmt/format.h"
// #include "ll/api/service/Bedrock.h"
// #include <ll/api/memory/Hook.h>
// #include <mc/entity/components/ProjectileComponent.h>
// #include <mc/server/ServerPlayer.h>
// #include <mc/world/ActorRuntimeID.h>
// #include <mc/world/item/EnderpearlItem.h>
// #include <mc/world/level/BlockSource.h>
// #include <mc/world/level/Level.h>
// #include <mc/world/level/Spawner.h>
// #include <mc/network/ServerNetworkHandler.h>

// namespace fh {

// bool start = false;

// LL_AUTO_TYPE_INSTANCE_HOOK(
//     DETECT_THROWN_ENDER_PEARL,
//     ll::memory::HookPriority::Normal,
//     Spawner,
//     "?spawnProjectile@Spawner@@QEAAPEAVActor@@AEAVBlockSource@@AEBUActorDefinitionIdentifier@@PEAV2@AEBVVec3@@3@Z",
//     Actor*,
//     BlockSource&                     region,
//     ActorDefinitionIdentifier const& id,
//     Actor*                           spawner,
//     Vec3 const&                      position,
//     Vec3 const&                      direction
// ) {
//     // if (spawner->isPlayer()) start = true;
//     return this->origin(region, id, spawner, position, direction);
// }

// LL_AUTO_TYPE_INSTANCE_HOOK(
//     ON_ENDER_PEARL_SHOOT,
//     ll::memory::HookPriority::Normal,
//     ProjectileComponent,
//     "?shoot@ProjectileComponent@@QEAAXAEAVActor@@AEBVVec3@@MM1PEAV2@@Z",
//     void,
//     Actor&      owner,
//     Vec3 const& dir,
//     float       pow,
//     float       uncertainty,
//     Vec3 const& baseSpeed,
//     Actor*      target
// ) {
//     if (start) {
//         ll::service::getLevel()->forEachPlayer([&](Player& player) -> bool {
//             player.sendMessage(fmt::format(
//                 "[Before shoot][{}] {} ---\n    pos {},\n    speed {},\n    speed length: {},\n    dir {},\n    pow "
//                 "{},\n    uncertainty {},\n    baseSpeed {}\n",
//                 owner.getDimensionBlockSource().getLevel().getCurrentTick().t,
//                 owner.getRuntimeID().id,
//                 owner.getPosition().toString(),
//                 owner.getPosDelta().toString(),
//                 owner.getPosDelta().length(),
//                 dir.toString(),
//                 pow,
//                 uncertainty,
//                 baseSpeed.toString()
//             ));
//             return true;
//         });
//     }
//     this->origin(owner, dir, pow, uncertainty, baseSpeed, target);
//     if (start) {
//         ll::service::getLevel()->forEachPlayer([&](Player& player) -> bool {
//             player.sendMessage(fmt::format(
//                 "[After shoot][{}] {} ---\n    pos {},\n    speed {},\n    speed length: {}\n",
//                 owner.getDimensionBlockSource().getLevel().getCurrentTick().t,
//                 owner.getRuntimeID().id,
//                 owner.getPosition().toString(),
//                 owner.getPosDelta().toString(),
//                 owner.getPosDelta().length()
//             ));
//             return true;
//         });
//         start = false;
//     }
// }

// LL_AUTO_TYPE_INSTANCE_HOOK(
//     TICK_ENDER_PEARL,
//     ll::memory::HookPriority::Normal,
//     ProjectileComponent,
//     &ProjectileComponent::handleMovementGravity,
//     void,
//     Actor& owner
// ) {
//     this->origin(owner);
//     // ll::service::getLevel()->forEachPlayer([&](Player& player) -> bool {
//     //     player.sendMessage(fmt::format(
//     //         "[Tick pearl][{}] {} ---\n    pos {},\n    speed {},\n    speed length: {}\n",
//     //         owner.getDimensionBlockSource().getLevel().getCurrentTick().t,
//     //         owner.getRuntimeID().id,
//     //         owner.getPosition().toString(),
//     //         owner.getPosDelta().toString(),
//     //         owner.getPosDelta().length()
//     //     ));
//     //     return true;
//     // });
// }

// } // namespace fh