#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_SEGMENT_SEGMENT_BUILDER_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_SEGMENT_SEGMENT_BUILDER_HPP

#include <functional>
#include <string>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <core/physical/segment/segment_context.h>

namespace sierra::odyplan::v3::core::physical::segment {
    class SegmentBuilder {
    public:
        static llvm::Function* createContinuation(llvm::Module& module, llvm::LLVMContext& ctx, const core::physical::types::CodegenTypeRegistry& codegenTypeRegistry,
            const std::string& name, const std::function<void(physical::segment::SegmentContext&, llvm::Value*)>& bodyFn);

        static void enqueueToComputeSegment(SegmentContext& segmentCtx, llvm::GlobalVariable* queueSlot, llvm::Function* processFn, llvm::Value* payload);
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_PHYSICAL_SEGMENT_SEGMENT_BUILDER_HPP