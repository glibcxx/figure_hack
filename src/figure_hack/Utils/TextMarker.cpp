#include "TextMarker.h"

#include <mc/server/ServerLevel.h>
#include <mc/server/commands/CommandUtils.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/ActorDefinitionIdentifier.h>


namespace fh {

std::vector<ActorUniqueID> Texts;

TextMarker::TextHandle TextMarker::addText(BlockSource& region, const std::string& text, const Vec3& pos) {
    ActorDefinitionIdentifier id;
    ActorUniqueID             uniqueid;
    id.initialize("fh:text_marker");
    Actor* textActor = CommandUtils::spawnEntityAt(region, pos, id, uniqueid, nullptr);
    if (!textActor) return {};
    textActor->setNameTag(text);
    Texts.push_back(uniqueid);
    return {};
}

void TextMarker::TextHandle::changeText(const std::string& text) {
    Actor* actor = this->mRegion->getLevel().fetchEntity(this->mActorId, false);
    if (actor) actor->setNameTag(text);
}

void TextMarker::TextHandle::changePos(const Vec3& pos) {
    Actor* actor = this->mRegion->getLevel().fetchEntity(this->mActorId, false);
    if (actor) actor->setPos(pos);
}

void TextMarker::TextHandle::change(const std::string& text, const Vec3& pos) {
    Actor* actor = this->mRegion->getLevel().fetchEntity(this->mActorId, false);
    if (actor) {
        actor->setNameTag(text);
        actor->setPos(pos);
    }
}

} // namespace fh