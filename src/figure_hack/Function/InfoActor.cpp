#include "Info.h"

#include <mc/world/actor/Actor.h>
#include <mc/world/actor/item/ItemActor.h>

namespace fh {

std::string buildSpecialActorDbgString(Actor& actor) {
    // todo
    switch (actor.getEntityTypeId()) {
    case ActorType::ItemEntity: {
        ItemActor& itemActor = static_cast<ItemActor&>(actor);
        return itemActor.item().toDebugString();
    }
    default: {
        std::string ret;
        actor.buildDebugInfo(ret);
        return ret;
    }
    }
}

} // namespace fh
