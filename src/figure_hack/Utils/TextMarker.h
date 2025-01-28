#pragma once

#include <mc/common/ActorUniqueID.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/world/level/BlockSource.h>


namespace fh {

class TextMarker {
public:
    class TextHandle {
        friend class TextMarker;

        BlockSource* const mRegion;
        ActorUniqueID      mActorId;
        TextHandle(BlockSource* region = nullptr, ActorUniqueID id = ActorUniqueID{-1})
        : mRegion(region),
          mActorId(id) {}

    public:
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