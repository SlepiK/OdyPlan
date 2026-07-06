#ifndef SIERRA_ODYPLAN_V2_CORE_LOGICAL_PLAN_PLAN_H
#define SIERRA_ODYPLAN_V2_CORE_LOGICAL_PLAN_PLAN_H

#include "core/common/ident.h"
#include "core/logical/operator/operator.h"
#include "core/common/edge/edge.h"
#include "core/common/port/port_ref.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace sierra::odyplan::v2::core::logical::plan {
    class Plan {
    public:
        template <typename T>
        common::identifier::OperatorId addOperator(std::unique_ptr<T> opr) {
            if (!opr) {
                throw std::invalid_argument("Plan::addOperator: opr must not be nullptr!");
            }

            const auto id = mOperators.size();

            if (!opr->hasInputPorts()) {
                mSourceOperators.push_back(id);
            }

            if constexpr (std::derived_from<T, opr::IBindable>) {
                mBindables[id] = opr.get();
            }

            mOperators.push_back(std::move(opr));
            return id;
        }

        void connect(const common::port::PortRef& fromOperator, const common::port::PortRef& toOperator);

        [[nodiscard]] std::vector<common::identifier::OperatorId> topologicalOrder() const;

        [[nodiscard]] opr::IOperator& op(common::identifier::OperatorId id) const;

    private:
        std::vector<common::identifier::OperatorId> mSourceOperators;
        std::vector<std::unique_ptr<opr::IOperator>> mOperators;
        std::vector<common::edge::Edge> mEdges;
        std::unordered_map<common::identifier::OperatorId, opr::IBindable*> mBindables;

        void validateConnectionRequest(const common::port::PortRef& fromOperator, const common::port::PortRef& toOperator);
        [[nodiscard]] bool areConnectionTypesCompatible(const common::port::PortRef& fromOperator, const common::port::PortRef& toOperator) const;
        [[nodiscard]] bool wouldCreateCycle(common::identifier::OperatorId from, common::identifier::OperatorId to) const;
    };
}  // namespace sierra::odyplan::v2::core::logical::port

#endif  // SIERRA_ODYPLAN_V2_CORE_LOGICAL_PLAN_PLAN_H