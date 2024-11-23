#pragma once

#include <string>

#include <mc/enums/CircuitComponentType.h>
#include <mc/world/ActorRuntimeID.h>
#include <mc/world/ActorUniqueID.h>
#include <mc/world/redstone/circuit/components/BaseCircuitComponent.h>


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

} // namespace fh