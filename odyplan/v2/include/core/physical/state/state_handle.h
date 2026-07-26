#ifndef SIERRA_ODYPLAN_V2_CORE_PHYSICAL_SATE_STATEHANDLE_H
#define SIERRA_ODYPLAN_V2_CORE_PHYSICAL_SATE_STATEHANDLE_H
#include <llvm/IR/GlobalVariable.h>

namespace sierra::odyplan::v2::core::physical::state {
    struct StateHandle {
        llvm::GlobalVariable* var = nullptr;
    };
}

#endif //SIERRA_ODYPLAN_V2_CORE_PHYSICAL_SATE_STATEHANDLE_H