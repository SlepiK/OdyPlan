#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_PLAN_PLAN_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_PLAN_PLAN_HPP

#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include <core/physical/operators/operator.h>
#include <core/physical/plan/detail/plan_construction.h>
#include <core/physical/segment/segment.h>
#include <core/shared/identifiers/operator_id.h>
#include <core/shared/operators/operator_port_reference.h>
#include <core/shared/operators/operator_port_reference_connection.h>
#include <memory>
#include <vector>

namespace sierra::odyplan::v3::core::physical::plan {
    class Plan {
    public:
        using OperatorId = shared::identifiers::OperatorId;
        using OperatorPortReference = shared::operators::OperatorPortReference;
        using OperatorPortReferenceConnection = shared::operators::OperatorPortReferenceConnection;
        using IOperator = operators::IOperator;

        explicit Plan(const types::CodegenTypeRegistry& codegenTypeRegistry);

        void addOperator(std::unique_ptr<IOperator> opr);

        void addSegment(OperatorId logicalOperatorId, segment::Segment segment);

        void addOutgoingEdge(OperatorId logicalOperatorId,
                             std::vector<OperatorPortReferenceConnection> edges);

        void codegen(llvm::Module& module, llvm::LLVMContext& ctx) const;

        OperatorId getSize() const;

    private:
        std::vector<std::unique_ptr<IOperator>> mOperators;
        std::unordered_map<OperatorId, segment::Segment> mSegments;
        std::unordered_map<OperatorId, std::vector<OperatorPortReferenceConnection>> mOutgoingEdges;
        std::vector<OperatorId> mOrder;
        const types::CodegenTypeRegistry* mCodegenTypeRegistry;

        static void codegenMain(llvm::Module& module,
                                llvm::LLVMContext& ctx,
                                const std::vector<llvm::Function*>& ioRoots,
                                const std::vector<llvm::Function*>& computeRoots,
                                llvm::Function* endPhase);
    };
}  // namespace sierra::odyplan::v3::core::physical::plan

#endif  // SIERRA_ODYPLAN_V3_CORE_PHYSICAL_PLAN_PLAN_HPP