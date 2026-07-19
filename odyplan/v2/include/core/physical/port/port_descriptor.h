#ifndef SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PORT_PORTDESCRIPTOR_H
#define SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PORT_PORTDESCRIPTOR_H

#include "core/common/identifier/type.h"

namespace sierra::odyplan::v2::core::physical::port {
    struct PortDescriptor {
        common::identifier::TypeId typeId;
        llvm::Type* llvmType;
    };
}  // namespace sierra::odyplan::v2::core::physical::port

#endif  // SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PORT_PORTDESCRIPTOR_H