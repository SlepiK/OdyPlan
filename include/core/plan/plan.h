#ifndef SIERRA_ODYPLAN_V1_CORE_PLAN_PLAN_H
#define SIERRA_ODYPLAN_V1_CORE_PLAN_PLAN_H

#include "core/operator/operator.h"
#include "core/operator/operator_handle.h"

#include <core/edge/edge.h>
#include <core/edge/typed_edge.h>
#include <memory>
#include <vector>

namespace sierra::odyplan::v1::core::plan {
    class Plan {
    public:
        Plan();

        template <typename OpT, typename... Args>
        auto add(Args&&... args) -> opr::OperatorHandle<typename OpT::Tin, typename OpT::TOut> {
            static_assert(std::is_base_of_v<opr::IOperator, OpT>);
            auto index = mOperators.size();
            mOperators.push_back(std::make_unique<OpT>(std::forward<Args>(args)...));
            return opr::OperatorHandle<typename OpT::TIn, typename OpT::TOut>(index);
        }

        template <tuple::Tuple TFromIn, tuple::Tuple T, tuple::Tuple TToOut>
        void connect(const opr::OperatorHandle<TFromIn, T>& fromHandle,
                     const opr::OperatorHandle<T, TToOut>& toHandle) {
            mEdges.push_back(
                std::make_unique<edge::TypedEdge<T>>(fromHandle.getIndex(), toHandle.getIndex()));
        }

    private:
        std::vector<std::unique_ptr<opr::IOperator>> mOperators;
        std::vector<std::unique_ptr<edge::Edge>> mEdges;
    };
}  // namespace sierra::odyplan::v1::core::plan

#endif  // SIERRA_ODYPLAN_V1_CORE_PLAN_PLAN_H