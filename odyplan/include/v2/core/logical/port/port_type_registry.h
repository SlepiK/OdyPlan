#ifndef SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORTTYPEREGISTRY_H
#define SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORTTYPEREGISTRY_H

#include <atomic>
#include "v2/core/identifier/identifier.h"

namespace sierra::odyplan::v2::core::logical::port {
    class PortTypeRegistry {
    public:
        using TypeId = std::size_t;

        PortTypeRegistry() {
            loadDefaults();
        }

        void registerType(TypeId id) {

        }

    private:
        std::unordered_map<TypeId, std::size_t> types;
        void loadDefaults() {

        }
    };
}  // namespace sierra::odyplan::v2::core::logical::port

#endif  // SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORTTYPEREGISTRY_H