#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_PLAN_PLAN_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_PLAN_PLAN_HPP

#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>

#include <memory>
#include <vector>

#include <core/physical/operators/operator.h>
#include <core/physical/segment/segment.h>
#include <core/physical/types/value_type_registry.h>
#include <core/physical/plan/detail/plan_construction.h>

#include <core/shared/identifiers/operator_id.h>
#include <core/shared/operators/operator_port_reference.h>
#include <core/shared/operators/operator_port_reference_connection.h>

namespace sierra::odyplan::v3::core::physical::plan {
    class Plan {
    public:
        using OperatorId = shared::identifiers::OperatorId;
        using OperatorPortReference = shared::operators::OperatorPortReference;
        using OperatorPortReferenceConnection = shared::operators::OperatorPortReferenceConnection;
        using IOperator = operators::IOperator;

        explicit Plan(const types::ValueTypeRegistry& valueTypeRegistry);

        void addOperator(std::unique_ptr<IOperator> opr);

        void addChain(OperatorId logicalOperatorId, segment::Segment chain);

        void addOutgoingEdge(OperatorId logicalOperatorId, std::vector<OperatorPortReferenceConnection> edges);

        void codegen(llvm::Module& module, llvm::LLVMContext& ctx) const;

    private:
        std::vector<std::unique_ptr<IOperator>> mOperators;
        std::unordered_map<OperatorId, segment::Segment> mChains;
        std::unordered_map<OperatorId, std::vector<OperatorPortReferenceConnection>> mOutgoingEdges;
        std::vector<OperatorId> mOrder;
        const types::ValueTypeRegistry* mValueTypeRegistry;

        static void codegenMain(llvm::Module& module, llvm::LLVMContext& ctx, const std::vector<llvm::Function*>& ioRoots, const std::vector<llvm::Function*>& computeRoots, llvm::Function* endPhase);
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_PHYSICAL_PLAN_PLAN_HPP