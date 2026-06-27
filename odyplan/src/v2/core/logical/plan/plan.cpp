#include "v2/core/logical/plan/plan.h"

namespace sierra::odyplan::v2::core::logical::plan {
    void Plan::validateConnectionRequest(const port::PortRef& fromOperator,
                                         const port::PortRef& toOperator) {

        //Verify no direct loop
        if (fromOperator.operatorId == toOperator.operatorId) {
            throw std::runtime_error("Self-loops are not allowed!");
        }

        //
    }

    bool Plan::areConnectionTypesCompatible(const port::PortRef& fromOperator, const port::PortRef& toOperator) {
        const auto& fromOp = mOperators.at(fromOperator.operatorId);
        const auto& toOp = mOperators.at(toOperator.operatorId);

        auto fromType = fromOp
    }
}  // namespace sierra::odyplan::v2::core::logical::plan