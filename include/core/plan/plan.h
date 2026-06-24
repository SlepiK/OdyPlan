#ifndef SIERRA_ODYPLAN_V1_CORE_PLAN_PLAN_H
#define SIERRA_ODYPLAN_V1_CORE_PLAN_PLAN_H

#include "core/operator/operator.h"
#include "core/operator/sourceoperator/source_operator.h"
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

            if constexpr (std::tuple_size_v<typename OpT::Tin> == 0) {
                mSourceOperators.push_back(index);
            }

            return opr::OperatorHandle<typename OpT::TIn, typename OpT::TOut>(index);
        }

        template <std::size_t Port = 0, typename FromOpt, tuple::Tuple TFromOut, tuple::Tuple... TFromIns, typename ToOpt, tuple::Tuple TToOut, tuple::Tuple... TToIns>
        void connect(const opr::OperatorHandle<FromOpt, TFromOut, TFromIns...>& fromHandle,
                     const opr::OperatorHandle<ToOpt, TToOut, TToIns...>& toHandle) {
            static_assert(Port < sizeof...(TToIns));
            static_assert(std::is_same_v<TFromOut, std::tuple_element_t<Port, std::tuple<TToIns...>>>);

            mEdges.push_back(
                std::make_unique<edge::TypedEdge<FromOpt, ToOpt, Port, TFromOut>>(fromHandle.getIndex(), toHandle.getIndex(), Port));
        }

    private:
        std::vector<std::unique_ptr<opr::IOperator>> mOperators;
        std::vector<std::size_t> mSourceOperators;
        std::vector<std::unique_ptr<edge::Edge>> mEdges;
    };
}  // namespace sierra::odyplan::v1::core::plan

#endif  // SIERRA_ODYPLAN_V1_CORE_PLAN_PLAN_H