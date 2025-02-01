#include <ll/api/memory/Hook.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/ActorDefinitionIdentifier.h>

namespace fh {

LL_AUTO_TYPE_INSTANCE_HOOK(
    NoSaveCustomActorData,
    ll::memory::HookPriority::Normal,
    Actor,
    &Actor::save,
    bool,
    CompoundTag& entityTag
) {
    if (this->getActorIdentifier().getNamespace() == "fh") return false;

    return this->origin(entityTag);
}

} // namespace fh