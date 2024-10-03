#pragma once

#include <ll/api/command/Command.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <ll/api/service/Bedrock.h>

#include "mc/enums/CircuitComponentType.h"
#include "mc/server/commands/CommandContext.h"
#include "mc/server/commands/MinecraftCommands.h"
#include "mc/server/commands/PlayerCommandOrigin.h"
#include "mc/world/Minecraft.h"
#include "mc/world/level/dimension/Dimension.h"

namespace fh {
namespace utils {

// without `/`
inline bool executeCommand(Player& executor, const std::string& command) {
    std::unique_ptr<PlayerCommandOrigin> origin = std::make_unique<PlayerCommandOrigin>(executor);
    CommandContext                       context{"/" + command, std::move(origin), CommandVersion::CurrentVersion};
    optional_ref<Minecraft>              mc = ll::service::getMinecraft();

    if (mc) {
        return mc->getCommands().executeCommand(context);
    } else {
        return false;
    }
}

inline const char* typeId2Name(CircuitComponentType type) {
    switch (type) {
    case CircuitComponentType::Unknown:
        return "Unknown";
    case CircuitComponentType::Undefined:
        return "Undefined";
    case CircuitComponentType::Mask:
        return "Mask";
    case CircuitComponentType::BaseCircuitComponent:
        return "BaseCircuit";
    case CircuitComponentType::BaseRailTransporter:
        return "Rail";
    case CircuitComponentType::ConsumerComponent:
        return "Consumer";
    case CircuitComponentType::PoweredBlockComponent:
        return "PoweredBlock";
    case CircuitComponentType::ProducerComponent:
        return "Producer";
    case CircuitComponentType::TransporterComponent:
        return "Transporter";
    case CircuitComponentType::CapacitorComponent:
        return "Capacitor";
    case CircuitComponentType::PistonConsumer:
        return "Piston";
    case CircuitComponentType::ComparatorCapacitor:
        return "Comparator";
    case CircuitComponentType::PulseCapacitor:
        return "Observer";
    case CircuitComponentType::RedstoneTorchCapacitor:
        return "Torch";
    case CircuitComponentType::RepeaterCapacitor:
        return "Repeater";
    default:
        return "";
    }
}

} // namespace utils

} // namespace fh
