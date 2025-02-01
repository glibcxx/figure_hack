#include "TextMarker.h"

#include <mc/network/SpatialActorNetworkData.h>
#include <mc/server/ServerLevel.h>
#include <mc/server/commands/CommandUtils.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/ActorDefinitionIdentifier.h>
#include <mc/world/phys/AABB.h>


namespace fh {

std::unordered_set<ActorUniqueID> Texts;

TextMarker::TextHandle TextMarker::addText(BlockSource& region, const std::string& text, const Vec3& pos) {
    ActorDefinitionIdentifier id;
    ActorUniqueID             uniqueid;
    id.initialize("fh:text_marker");
    Actor* textActor = CommandUtils::spawnEntityAt(region, pos, id, uniqueid, nullptr);
    if (!textActor) return {region};
    textActor->setNameTag(text);
    textActor->enableAutoSendPosRot(true);
    textActor->setAABB(AABB{0, 0, 0, 0, 0, 0});
    Texts.emplace(uniqueid);
    return {region, uniqueid};
}

void TextMarker::TextHandle::changeText(const std::string& text) {
    Actor* actor = this->mLevel.fetchEntity(this->mActorId, false);
    if (actor) {
        actor->setNameTag(text);
        actor->_sendDirtyActorData();
    }
}

void TextMarker::TextHandle::changePos(const Vec3& pos) {
    Actor* actor = this->mLevel.fetchEntity(this->mActorId, false);
    if (actor) {
        actor->moveTo(pos, Vec2{});
        actor->_sendDirtyActorData();
        actor->getSpatialNetworkData().sendUpdate(false, false, false);
    }
}

void TextMarker::TextHandle::change(const std::string& text, const Vec3& pos) {
    Actor* actor = this->mLevel.fetchEntity(this->mActorId, false);
    if (actor) {
        actor->setNameTag(text);
        actor->moveTo(pos, Vec2{});
        actor->_sendDirtyActorData();
        actor->getSpatialNetworkData().sendUpdate(false, false, false);
    }
}

void TextMarker::TextHandle::remove() {
    if (this->mActorId.rawID == -1) return;
    Actor* actor = this->mLevel.fetchEntity(this->mActorId, false);
    if (actor) {
        actor->remove();
        actor->_sendDirtyActorData();
        actor->getSpatialNetworkData().sendUpdate(false, false, false);
    }
    Texts.erase(this->mActorId);
    this->mActorId = {};
}

} // namespace fh