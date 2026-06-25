#ifndef SIERRA_ODYPLAN_V1_CORE_PLAN_PLAN_H
#define SIERRA_ODYPLAN_V1_CORE_PLAN_PLAN_H

#include "core/operator/operator.h"
#include "core/operator/operator_handle.h"
#include "core/operator/pipeoperator/pipe_operator_handle.h"
#include "core/operator/sinkoperator/sink_operator_handle.h"
#include "core/operator/sourceoperator/source_operator_handle.h"

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

        //Source -> Pipe & Pipe -> Pipe
        template <std::size_t PortIndex, typename From, opr::PipeOperatorHandleType To>
            requires (opr::SourceOperatorHandleType<From> || opr::PipeOperatorHandleType<From>)
        void connect(const From& from, const To& to) {
            connectImpl<PortIndex>(from, to);
        }

        //Source -> Sink & Pipe -> Sink
        template <typename From, opr::SinkOperatorHandleType To>
            requires (opr::SourceOperatorHandleType<From> || opr::PipeOperatorHandleType<From>)
        void connect(const From& from, const To& to) {
            connectImpl<0>(from, to);
        }

        //Source
        template <typename OperatorType, typename ... Args>
            requires (opr::SourceOperatorType<OperatorType>)
        auto add(Args&&... args) -> opr::SourceOperatorHandle<OperatorType> {
            auto index = mOperators.size();
            mOperators.push_back(std::make_unique<OperatorType>(std::forward<Args>(args)...));
            mSourceOperators.push_back(index);
            return opr::SourceOperatorHandle<OperatorType>(index);
        }

        //Pipe
        template <typename OperatorType, typename ... Args>
            requires (opr::PipeOperatorType<OperatorType>)
        auto add(Args&&... args) -> opr::PipeOperatorHandle<OperatorType> {
            auto index = mOperators.size();
            mOperators.push_back(std::make_unique<OperatorType>(std::forward<Args>(args)...));
            return opr::PipeOperatorHandle<OperatorType>(index);
        }

        //Sink
        template <typename OperatorType, typename ... Args>
            requires (opr::SinkOperatorType<OperatorType>)
        auto add(Args&&... args) -> opr::SinkOperatorHandle<OperatorType> {
            auto index = mOperators.size();
            mOperators.push_back(std::make_unique<OperatorType>(std::forward<Args>(args)...));
            return opr::SinkOperatorHandle<OperatorType>(index);
        }


        /*template <typename OpT, typename... Args>
        auto add(Args&&... args) -> opr::HandleFromOperator<OpT> {
            static_assert(std::is_base_of_v<opr::IOperator, OpT>);
            auto index = mOperators.size();
            mOperators.push_back(std::make_unique<OpT>(std::forward<Args>(args)...));

            if constexpr (std::is_base_of_v<opr::ISourceOperator, OpT>) {
                mSourceOperators.push_back(index);
            }

            return opr::HandleFromOperator<OpT>(index);
        }*/

    private:
        std::vector<std::unique_ptr<opr::IOperator>> mOperators;
        std::vector<std::size_t> mSourceOperators;
        std::vector<std::unique_ptr<edge::Edge>> mEdges;

        template <std::size_t PortIndex, typename From, typename To>
        void connectImpl(const From& from, const To& to) {
            using FromOut = typename From::Output;
            using ToIn = typename To::Input;

            if constexpr (opr::SinkOperatorHandleType<To>) {
                static_assert(port::PortType<FromOut>);
                static_assert(port::PortType<ToIn>);

                connectPorts<PortIndex>(from, to);

            } else if constexpr (opr::PipeOperatorHandleType<To>) {
                static_assert(PortIndex < To::SIZE);
                using TargetPort = std::tuple_element_t<PortIndex, typename To::PortEntries>;
                static_assert(std::is_same_v<FromOut, TargetPort>);

                connectPorts<PortIndex>(from, to);
            }
        }

        template <std::size_t PortIndex, typename From, typename To>
        void connectPorts(const From& from, const To& to) {
            mEdges.push_back(std::make_unique<edge::TypedEdge<typename From::OperatorType, typename To::OperatorType, PortIndex>>(from.index(), to.index()));
        }
    };
}  // namespace sierra::odyplan::v1::core::plan

#endif  // SIERRA_ODYPLAN_V1_CORE_PLAN_PLAN_H