#include "TextMarker.h"

#include <mc/network/SpatialActorNetworkData.h>
#include <mc/network/packet/AddActorPacket.h>
#include <mc/server/ServerLevel.h>
#include <mc/server/commands/CommandUtils.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/ActorDefinitionIdentifier.h>
#include <mc/world/phys/AABB.h>


namespace fh {

std::unordered_set<ActorUniqueID> Texts;

TextMarker::TextObj
TextMarker::addText(BlockSource& region, const std::string& text, const Vec3& pos, bool syncToClientImmediatly) {
    ActorDefinitionIdentifier id;
    ActorUniqueID             uniqueid;
    id.initialize("fh:text_marker");
    Actor* textActor = CommandUtils::spawnEntityAt(region, pos, id, uniqueid, nullptr);
    if (!textActor) return {region};
    textActor->setNameTag(text);
    textActor->setAABB(AABB{0, 0, 0, 0, 0, 0});
    if (syncToClientImmediatly) {
        auto packet = textActor->tryCreateAddActorPacket();
        if (packet) packet->sendToClients();
    }
    return {region, uniqueid};
}

void TextMarker::TextObj::changeText(const std::string& text, bool syncToClientImmediatly) {
    Actor* actor = this->mLevel.fetchEntity(this->mActorId, false);
    if (actor) {
        actor->setNameTag(text);
        if (syncToClientImmediatly) actor->_sendDirtyActorData();
    }
}

void TextMarker::TextObj::changePos(const Vec3& pos, bool syncToClientImmediatly) {
    Actor* actor = this->mLevel.fetchEntity(this->mActorId, false);
    if (actor) {
        actor->moveTo(pos, Vec2{});
        if (syncToClientImmediatly) {
            actor->_sendDirtyActorData();
            actor->getSpatialNetworkData().sendUpdate(false, false, false);
        }
    }
}

void TextMarker::TextObj::change(const std::string& text, const Vec3& pos, bool syncToClientImmediatly) {
    Actor* actor = this->mLevel.fetchEntity(this->mActorId, false);
    if (actor) {
        actor->setNameTag(text);
        actor->moveTo(pos, Vec2{});
        if (syncToClientImmediatly) {
            actor->_sendDirtyActorData();
            actor->getSpatialNetworkData().sendUpdate(false, false, false);
        }
    }
}

void TextMarker::TextObj::remove() {
    if (this->mActorId.rawID == -1) return;
    Actor* actor = this->mLevel.fetchEntity(this->mActorId, false);
    if (actor) actor->remove();
    Texts.erase(this->mActorId);
    this->mActorId = {};
}

} // namespace fh