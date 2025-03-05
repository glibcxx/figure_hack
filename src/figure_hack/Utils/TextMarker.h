#pragma once

#include <mc/deps/core/math/Vec2.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/world/level/BlockSource.h>


namespace fh {

class TextMarker {
public:
    class TextObj {
        friend class TextMarker;

        Level&        mLevel;
        ActorUniqueID mActorId;
        TextObj(BlockSource& region, ActorUniqueID id = {}) : mLevel(region.getLevel()), mActorId(id) {}

    public:
        TextObj(const TextObj&)            = delete;
        TextObj& operator=(const TextObj&) = delete;

        TextObj(TextObj&& other) : mLevel(other.mLevel), mActorId(other.mActorId) { other.mActorId = {}; }

        ~TextObj() {
            if (this->mActorId.rawID != -1) {
                this->remove();
            }
        }

        void changeText(const std::string& text, bool syncToClientImmediatly = false);
        void changePos(const Vec3& pos, bool syncToClientImmediatly = false);
        void change(const std::string& text, const Vec3& pos, bool syncToClientImmediatly = false);
        void remove();

        bool isValid() const { return this->mActorId.rawID != -1; }
        operator bool() const { return this->mActorId.rawID != -1; }
    };

    static TextObj
    addText(BlockSource& region, const std::string& text, const Vec3& pos, bool syncToClientImmediatly = false);
};

} // namespace fh