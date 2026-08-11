#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_STATE_STATE_HANDLE_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_STATE_STATE_HANDLE_HPP

#include <llvm/IR/GlobalVariable.h>

namespace sierra::odyplan::v3::core::physical::state {
    struct StateHandle {
        llvm::GlobalVariable* var = nullptr;
    };
}  // namespace sierra::odyplan::v3::core::physical::state

#endif  // SIERRA_ODYPLAN_V3_CORE_PHYSICAL_STATE_STATE_HANDLE_HPP