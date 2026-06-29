#ifndef SIERRA_ODYPLAN_V2_CORE_LOGICAL_PLAN_PLAN_H
#define SIERRA_ODYPLAN_V2_CORE_LOGICAL_PLAN_PLAN_H

#include "v2/core/logical/operator/operator.h"
#include "v2/core/logical/edge/edge.h"
#include "v2/core/logical/port/port_ref.h"

#include <memory>
#include <vector>

namespace sierra::odyplan::v2::core::logical::plan {
    class Plan {
    public:
        port::OperatorId addOperator(std::unique_ptr<opr::IOperator> opr);

        template <typename T>
        void connect(const port::PortRef& fromOperator, const port::PortRef& toOperator) {
            validateConnectionRequest(fromOperator, toOperator);

            if (!areConnectionTypesCompatible(fromOperator,toOperator)) {
                throw std::runtime_error("Plan::connect: source and destination port type dont match");
            }

            //ToDo: add more
        };

    protected:
        opr::IOperator& op(port::OperatorId id) {
            return *mOperators.at(id);
        }

    private:
        std::vector<std::unique_ptr<opr::IOperator>> mOperators;
        std::vector<edge::Edge> mEdges;

        void validateConnectionRequest(const port::PortRef& fromOperator, const port::PortRef& toOperator);
        bool areConnectionTypesCompatible(const port::PortRef& fromOperator, const port::PortRef& toOperator);
    };
}  // namespace sierra::odyplan::v2::core::logical::port

#endif  // SIERRA_ODYPLAN_V2_CORE_LOGICAL_PLAN_PLAN_H