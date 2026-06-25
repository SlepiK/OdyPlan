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
        Plan() = default;
        ~Plan() = default;
        Plan(const Plan&) = delete;
        Plan& operator=(const Plan&) = delete;
        Plan(Plan&&) = default;
        Plan& operator=(Plan&&) = default;

        template <typename OpT, typename... Args>
        auto add(Args&&... args) -> opr::HandleFromOperator<OpT> {
            static_assert(std::is_base_of_v<opr::IOperator, OpT>);
            auto index = mOperators.size();
            mOperators.push_back(std::make_unique<OpT>(std::forward<Args>(args)...));

            if constexpr (std::is_base_of_v<opr::ISourceOperator, OpT>) {
                mSourceOperators.push_back(index);
            }

            return opr::HandleFromOperator<OpT>(index);
        }

        template <std::size_t Port = 0, typename FromOpt, port::PortType TFromOut, port::PortsType TFromIns, typename ToOpt, port::PortType TToOut, port::PortsType TToIns>
        void connect(const opr::OperatorHandle<FromOpt, TFromOut, TFromIns>& fromHandle,
                     const opr::OperatorHandle<ToOpt, TToOut, TToIns>& toHandle) {

            using TargetPort = std::tuple_element_t<Port, typename TToIns::PortEntries>;

            static_assert(Port < TToIns::SIZE);
            static_assert(std::is_same_v<TFromOut, TargetPort>);

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