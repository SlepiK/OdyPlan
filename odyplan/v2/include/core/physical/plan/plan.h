#ifndef SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PLAN_PLAN_H
#define SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PLAN_PLAN_H

#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>

#include "core/physical/operator/operator.h"
#include "core/physical/segment/segment.h"

#include <memory>
#include <vector>

#include "core/common/type/type_registry.h"

namespace sierra::odyplan::v2::core::physical::plan {
    class Plan {
    public:
        explicit Plan(const common::type::TypeRegistry& typeRegistry)
            : mTypeRegistry(typeRegistry) {
        }

        void addOperator(std::unique_ptr<opr::IOperator> opr);

        void addSegment(segment::Segment segment);

        void codegen(llvm::Module& module, llvm::LLVMContext& ctx);

    private:
        std::vector<std::unique_ptr<opr::IOperator>> mOperators;
        std::vector<segment::Segment> mSegments;
        const common::type::TypeRegistry& mTypeRegistry;

        llvm::Function* codegenSegment(const segment::Segment& segment, llvm::Module& module, llvm::LLVMContext& ctx, std::size_t index);

        void codegenMain(llvm::Module& module, llvm::LLVMContext& ctx, const std::vector<llvm::Function*>& functions);
    };
}  // namespace sierra::odyplan::v2::core::physical::port

#endif  // SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PLAN_PLAN_H