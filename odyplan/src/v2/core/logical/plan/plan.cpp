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

        //Verify that each operator can exist, by using the operatorId
        if (fromOperator.operatorId.value >= mOperators.size()) {
            throw std::out_of_range("Plan::connect: fromOperator.operatorId out of range");
        }

        if (toOperator.operatorId.value >= mOperators.size()) {
            throw std::out_of_range("Plan::connect: toOperator.operatorId out of range");
        }

        if (const auto& fromOuts = op(fromOperator.operatorId).outputs(); fromOperator.portId.value >= fromOuts.size()) {
            throw std::out_of_range("Plan::connect: fromOperator.portId out of range");
        }

        if (const auto& toIns = op(fromOperator.operatorId).inputs(); toOperator.portId.value >= toIns.size()) {
            throw std::out_of_range("Plan::connect: toOperator.portId out of range");
        }
    }

    bool Plan::areConnectionTypesCompatible(const port::PortRef& fromOperator, const port::PortRef& toOperator) {
        const auto& fromOuts = op(fromOperator.operatorId).outputs();
        const auto& toIns = op(toOperator.operatorId).inputs();

        if (fromOperator.portId.value >= fromOuts.size() || toOperator.portId.value >= toIns.size()) {
            throw std::out_of_range("Plan::areConnectionTypesCompatible: port index out of range");
        }

        return fromOuts.operator[](fromOperator.portId.value).id == toIns.operator[](toOperator.portId.value).id;
    }
}  // namespace sierra::odyplan::v2::core::logical::plan