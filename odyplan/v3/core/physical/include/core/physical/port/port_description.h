#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_PORT_PORT_DESCRIPTION_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_PORT_PORT_DESCRIPTION_HPP

#include <llvm/IR/Type.h>

#include <core/shared/types/value_type_id.h>

namespace sierra::odyplan::v3::core::physical::port {
    struct PortDescription {
        using ValueTypeId = shared::types::ValueTypeId;

        ValueTypeId valueTypeId;
        llvm::Type* llvmType;
    };
}  // namespace sierra::odyplan::v3::core::physical::port

#endif  // SIERRA_ODYPLAN_V3_CORE_PHYSICAL_PORT_PORT_DESCRIPTION_HPP