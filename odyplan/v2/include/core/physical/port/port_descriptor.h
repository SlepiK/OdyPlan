#ifndef SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PORT_PORTDESCRIPTOR_H
#define SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PORT_PORTDESCRIPTOR_H

#include <llvm/IR/Type.h>
#include "core/common/ident.h"
#include "../../common/type/value/type.h"

namespace sierra::odyplan::v2::core::physical::port {
    struct PortDescriptor {
        common::type::value::TypeId typeId;
        llvm::Type* llvmType;
    };
}  // namespace sierra::odyplan::v2::core::physical::port

#endif  // SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PORT_PORTDESCRIPTOR_H