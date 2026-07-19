#ifndef SIERRA_ODYPLAN_V2_CORE_PHYSICAL_SEGMENT_SEGMENTCONTEXT_H
#define SIERRA_ODYPLAN_V2_CORE_PHYSICAL_SEGMENT_SEGMENTCONTEXT_H

#include <llvm/IR/IRBuilder.h>

#include "core/common/storage/storage.h"

namespace sierra::odyplan::v2::core::physical::segment {
    struct SegmentContext {
        llvm::Function* function;
        llvm::IRBuilder<> builder;
        common::storage::Storage storage;
        const common::type::TypeRegistry& typeRegistry;

        SegmentContext(llvm::Function* fn, llvm::BasicBlock* block, llvm::LLVMContext& ctx, const common::type::TypeRegistry& typeRegistry)
            : function(fn),
                builder(block),
                storage(*fn, ctx, typeRegistry),
                typeRegistry(typeRegistry) {
        }
    };
}  // namespace sierra::odyplan::v2::core::physical::segment

#endif  // SIERRA_ODYPLAN_V2_CORE_PHYSICAL_SEGMENT_SEGMENTCONTEXT_H