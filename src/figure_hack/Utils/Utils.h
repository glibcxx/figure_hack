#pragma once

#include <ll/api/command/Command.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <ll/api/i18n/I18n.h>
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

inline std::string typeId2Name(CircuitComponentType type) {
    using ll::i18n_literals::operator""_tr;
    switch (type) {
    case CircuitComponentType::Unknown:
        return "restone_type_name.unknown"_tr();
    case CircuitComponentType::Undefined:
        return "restone_type_name.undefined"_tr();
    case CircuitComponentType::Mask:
        return "restone_type_name.mask"_tr();
    case CircuitComponentType::BaseCircuitComponent:
        return "restone_type_name.base_circuit"_tr();
    case CircuitComponentType::BaseRailTransporter:
        return "restone_type_name.rail"_tr();
    case CircuitComponentType::ConsumerComponent:
        return "restone_type_name.consumer"_tr();
    case CircuitComponentType::PoweredBlockComponent:
        return "restone_type_name.powered_block"_tr();
    case CircuitComponentType::ProducerComponent:
        return "restone_type_name.producer"_tr();
    case CircuitComponentType::TransporterComponent:
        return "restone_type_name.transporter"_tr();
    case CircuitComponentType::CapacitorComponent:
        return "restone_type_name.capacitor"_tr();
    case CircuitComponentType::PistonConsumer:
        return "restone_type_name.piston"_tr();
    case CircuitComponentType::ComparatorCapacitor:
        return "restone_type_name.comparator"_tr();
    case CircuitComponentType::PulseCapacitor:
        return "restone_type_name.observer"_tr();
    case CircuitComponentType::RedstoneTorchCapacitor:
        return "restone_type_name.torch"_tr();
    case CircuitComponentType::RepeaterCapacitor:
        return "restone_type_name.repeater"_tr();
    default:
        return "restone_type_name.unknown"_tr();;
    }
}

} // namespace utils

} // namespace fh
