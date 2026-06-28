#include "v2/core/logical/plan/plan.h"

namespace sierra::odyplan::v2::core::logical::plan {

    port::OperatorId Plan::addOperator(std::unique_ptr<opr::IOperator> opr) {
        if (!opr) {
            throw std::invalid_argument("Plan::addOperator: opr must not be nullptr!");
        }

        mOperators.push_back(std::move(opr));
        return port::OperatorId(mOperators.size() - 1);
    }

    void Plan::validateConnectionRequest(const port::PortRef& fromOperator,
                                         const port::PortRef& toOperator) {

        //Verify no direct loop
        if (fromOperator.operatorId == toOperator.operatorId) {
            throw std::runtime_error("Self-loops are not allowed!");
        }

        //
    }

    bool Plan::areConnectionTypesCompatible(const port::PortRef& fromOperator, const port::PortRef& toOperator) {
        const auto& fromOp = *mOperators.at(fromOperator.operatorId);
        const auto& toOp = *mOperators.at(toOperator.operatorId);

        const auto& fromOutputs = fromOp.outputs();
        const auto& toInputs = toOp.inputs();

        if (fromOperator.portId >= fromOperator.size()) {
            return false;
        }

        if (toOperator.portId >= toOperator.size()) {
            return false;
        }

        return fromOutputs[fromOperator.portId].id == toInputs[toOperator.portId].id;
    }
}  // namespace sierra::odyplan::v2::core::logical::plan