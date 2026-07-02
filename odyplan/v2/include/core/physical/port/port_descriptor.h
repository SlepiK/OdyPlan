#ifndef SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PORT_PORTDESCRIPTOR_H
#define SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PORT_PORTDESCRIPTOR_H

#include <llvm/IR/Type.h>
#include "core/logical/port/port_descriptor.h"

namespace sierra::odyplan::v2::core::physical::port {
    struct PortDescriptor {
        logical::port::PortTypeId typeId;
        llvm::Type* llvmType;
    };
}  // namespace sierra::odyplan::v2::core::physical::port

#endif  // SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PORT_PORTDESCRIPTOR_H