#ifndef SIERRA_ODYPLAN_V2_CORE_LOGICAL_PLAN_PLAN_H
#define SIERRA_ODYPLAN_V2_CORE_LOGICAL_PLAN_PLAN_H

#include "core/logical/operator/operator.h"
#include "core/logical/edge/edge.h"
#include "core/logical/port/port_ref.h"

#include <memory>
#include <vector>

namespace sierra::odyplan::v2::core::logical::plan {
    class Plan {
    public:
        port::OperatorId addOperator(std::unique_ptr<opr::IOperator> opr);

        void connect(const port::PortRef& fromOperator, const port::PortRef& toOperator);

    protected:
        [[nodiscard]] opr::IOperator& op(port::OperatorId id) const;

    private:
        std::vector<port::OperatorId> mSourceOperators;
        std::vector<std::unique_ptr<opr::IOperator>> mOperators;
        std::vector<edge::Edge> mEdges;

        void validateConnectionRequest(const port::PortRef& fromOperator, const port::PortRef& toOperator);
        [[nodiscard]] bool areConnectionTypesCompatible(const port::PortRef& fromOperator, const port::PortRef& toOperator) const;
        [[nodiscard]] bool wouldCreateCycle(port::OperatorId from, port::OperatorId to) const;
    };
}  // namespace sierra::odyplan::v2::core::logical::port

#endif  // SIERRA_ODYPLAN_V2_CORE_LOGICAL_PLAN_PLAN_H