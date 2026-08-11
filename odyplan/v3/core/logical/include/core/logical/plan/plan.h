#ifndef SIERRA_ODYPLAN_V3_CORE_LOGICAL_PLAN_PLAN_HPP
#define SIERRA_ODYPLAN_V3_CORE_LOGICAL_PLAN_PLAN_HPP

#include <core/logical/operators/bindable/bindable_operator.h>
#include <core/logical/operators/operator.h>
#include <core/shared/identifiers/operator_id.h>
#include <core/shared/operators/operator_port_reference.h>
#include <core/shared/operators/operator_port_reference_connection.h>
#include <memory>
#include <unordered_map>
#include <vector>

namespace sierra::odyplan::v3::core::logical::plan {
    class Plan {
    public:
        using OperatorId = shared::identifiers::OperatorId;
        using OperatorPortReference = shared::operators::OperatorPortReference;
        using OperatorPortReferenceConnection = shared::operators::OperatorPortReferenceConnection;
        using IBindableOperator = operators::bindable::IBindableOperator;
        using IOperator = operators::IOperator;

        template <typename T>
        OperatorId addOperator(std::unique_ptr<T> opr) {
            if (!opr) {
                throw std::invalid_argument("Plan::addOperator: opr must not be nullptr!");
            }

            const auto id = mOperators.size();

            if (!opr->hasInputPorts()) {
                mSourceOperators.push_back(id);
            }

            if constexpr (std::derived_from<T, IBindableOperator>) {
                mBindables[id] = opr.get();
            }

            mOperators.push_back(std::move(opr));
            return id;
        }

        void connect(const OperatorPortReference& fromOperator,
                     const OperatorPortReference& toOperator);

        [[nodiscard]] std::vector<OperatorId> topologicalOrder() const;

        [[nodiscard]] IOperator& op(OperatorId id) const;

        [[nodiscard]] std::vector<OperatorPortReferenceConnection>
        getInEdgesFromOperator(OperatorId id) const;

        [[nodiscard]] std::vector<OperatorPortReferenceConnection>
        getOutEdgesFromOperator(OperatorId id) const;
    private:
        std::vector<OperatorId> mSourceOperators;
        std::vector<std::unique_ptr<IOperator>> mOperators;
        std::vector<OperatorPortReferenceConnection> mEdges;
        std::unordered_map<OperatorId, IBindableOperator*> mBindables;
        std::unordered_map<OperatorId, std::vector<OperatorPortReferenceConnection>> mIncomingEdges;
        std::unordered_map<OperatorId, std::vector<OperatorPortReferenceConnection>> mOutgoingEdges;

        void validateConnectionRequest(const OperatorPortReference& fromOperator,
                                       const OperatorPortReference& toOperator);

        [[nodiscard]] bool
        areConnectionTypesCompatible(const OperatorPortReference& fromOperator,
                                     const OperatorPortReference& toOperator) const;

        [[nodiscard]] bool wouldCreateCycle(OperatorId from, OperatorId to) const;
    };
}  // namespace sierra::odyplan::v3::core::logical::plan

#endif  // SIERRA_ODYPLAN_V3_CORE_LOGICAL_PLAN_PLAN_HPP