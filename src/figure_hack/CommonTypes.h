#pragma once

#include <string>

#include <mc/common/ActorRuntimeID.h>
#include <mc/common/ActorUniqueID.h>
#include <mc/world/redstone/circuit/components/BaseCircuitComponent.h>
#include <mc/world/redstone/circuit/components/CircuitComponentType.h>

namespace fh {
struct BlockInfo {
    std::string name;
};

struct CircuitInfo {
    CircuitComponentType  typeId;
    BaseCircuitComponent* rawPtr;
    CircuitComponentList& sources;
    int                   strength;
};

struct ActorInfo {
    const std::string& typeId;
    ActorRuntimeID     runtimeId;
    const Vec3&        pos;
    const Vec3&        posPrev;
    const Vec3&        velocity;
};

enum class ActorInfoMode : uint8_t { toggle = 0, overall, type, movement, status, special };

} // namespace fh