#include "TextMarker.h"

#include <mc/deps/ecs/gamerefs_entity/GameRefsEntity.h>
#include <mc/network/SpatialActorNetworkData.h>
#include <mc/network/packet/AddActorPacket.h>
#include <mc/server/ServerLevel.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/ActorDefinitionIdentifier.h>
#include <mc/world/actor/ActorFactory.h>

namespace fh {

TextMarker::TextObj
TextMarker::addText(BlockSource& region, const std::string& text, const Vec3& pos, bool syncToClientImmediatly) {
    ActorDefinitionIdentifier id;
    id.initialize("fh:text_marker");
    auto   actor     = region.getLevel().getActorFactory().createSpawnedActor(id, nullptr, pos, {});
    Actor* textActor = region.getLevel().addEntity(region, actor);
    if (!textActor) return {region};
    textActor->setNameTag(text);
    if (syncToClientImmediatly) {
        auto packet = textActor->tryCreateAddActorPacket();
        if (packet) packet->sendToClients();
    }
    return {region, textActor->getOrCreateUniqueID()};
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
            actor->mNetworkData->sendUpdate(false, false, false);
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
            actor->mNetworkData->sendUpdate(false, false, false);
        }
    }
}

void TextMarker::TextObj::remove() {
    if (this->mActorId.rawID == -1) return;
    Actor* actor = this->mLevel.fetchEntity(this->mActorId, false);
    if (actor) actor->remove();
    this->mActorId = {};
}

} // namespace fh