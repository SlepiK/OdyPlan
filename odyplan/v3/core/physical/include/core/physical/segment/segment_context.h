#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_SEGMENT_SEGMENT_CONTEXT_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_SEGMENT_SEGMENT_CONTEXT_HPP

#include <llvm/IR/IRBuilder.h>

#include <core/physical/storage/storage.h>
#include <core/physical/types/value_type_registry.h>

namespace sierra::odyplan::v3::core::physical::segment {
    struct SegmentContext {
        llvm::Function* function;
        llvm::IRBuilder<> builder;
        storage::Storage storage;
        const types::ValueTypeRegistry& valueTypeRegistry;

        SegmentContext(llvm::Function* fn,
                       llvm::BasicBlock* block,
                       llvm::LLVMContext& ctx,
                       const types::ValueTypeRegistry& valueTypeRegistry)
            : function(fn),
              builder(block),
              storage(*fn, ctx, valueTypeRegistry),
              valueTypeRegistry(valueTypeRegistry) {}
    };
}  // namespace sierra::odyplan::v3::core::physical::segment

#endif  // SIERRA_ODYPLAN_V3_CORE_PHYSICAL_SEGMENT_SEGMENT_CONTEXT_HPP