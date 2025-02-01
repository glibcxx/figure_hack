#pragma once

#include <mc/common/ActorUniqueID.h>
#include <mc/deps/core/math/Vec2.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/world/level/BlockSource.h>


namespace fh {

class TextMarker {
public:
    class TextHandle {
        friend class TextMarker;

        Level&        mLevel;
        ActorUniqueID mActorId;
        TextHandle(BlockSource& region, ActorUniqueID id = {}) : mLevel(region.getLevel()), mActorId(id) {}

    public:
        TextHandle(TextHandle&& other) : mLevel(other.mLevel), mActorId(other.mActorId) { other.mActorId = {}; }

        ~TextHandle() {
            if (this->mActorId.rawID != -1) {
                this->remove();
            }
        }

        void changeText(const std::string& text);
        void changePos(const Vec3& pos);
        void change(const std::string& text, const Vec3& pos);
        void remove();

        bool isValid() const { return this->mActorId.rawID != -1; }
        operator bool() const { return this->mActorId.rawID != -1; }
    };

    static TextHandle addText(BlockSource& region, const std::string& text, const Vec3& pos);
};

} // namespace fh