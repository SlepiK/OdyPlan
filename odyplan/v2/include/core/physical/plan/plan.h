#ifndef SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PLAN_PLAN_H
#define SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PLAN_PLAN_H

#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>

#include "core/physical/operator/operator.h"
#include "core/physical/segment/segment.h"

#include <memory>
#include <vector>

#include "core/common/port/port_ref.h"
#include "core/common/type/type_registry.h"
#include "core/physical/chain/chain.h"
#include "core/physical/edge/edge.h"

namespace sierra::odyplan::v2::core::physical::plan {
    class Plan {
    public:
        explicit Plan(const common::type::TypeRegistry& typeRegistry)
            : mTypeRegistry(&typeRegistry) {
        }

        void addOperator(std::unique_ptr<opr::IOperator> opr);

        void addChain(common::identifier::OperatorId logicalOperatorId, chain::Chain chain);

        void addOutgoingEdge(common::identifier::OperatorId logicalOperatorId, std::vector<edge::Edge> edges);

        void codegen(llvm::Module& module, llvm::LLVMContext& ctx) const;

    private:
        std::vector<std::unique_ptr<opr::IOperator>> mOperators;
        std::unordered_map<common::identifier::OperatorId, chain::Chain> mChains;
        std::unordered_map<common::identifier::OperatorId, std::vector<edge::Edge>> mOutgoingEdges;
        std::vector<common::identifier::OperatorId> mOrder;
        const common::type::TypeRegistry* mTypeRegistry;

        static void codegenMain(llvm::Module& module, llvm::LLVMContext& ctx, const std::vector<llvm::Function*>& ioRoots, const std::vector<llvm::Function*>& computeRoots, llvm::Function* endPhase);
    };
}  // namespace sierra::odyplan::v2::core::physical::port

#endif  // SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PLAN_PLAN_H