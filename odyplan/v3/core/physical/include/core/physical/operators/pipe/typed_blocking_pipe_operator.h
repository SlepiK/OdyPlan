#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_OPERATORS_PIPE_TYPED_BLOCKING_PIPE_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_OPERATORS_PIPE_TYPED_BLOCKING_PIPE_OPERATOR_HPP

#include <core/physical/flow/flow_producer.h>
#include <core/physical/operators/pipe/typed_pipe_operator.h>

namespace sierra::odyplan::v3::core::physical::operators::pipe {
    class TypedBlockingPipeOperator : public TypedPipeOperator, public flow::IProducer {
    public:
        explicit TypedBlockingPipeOperator(port::Ports inputPorts, port::Ports outputPorts);

        [[nodiscard]] bool isSegmentBreaker() const final;

        void consume(segment::SegmentContext& segmentCtx,
                     shared::identifiers::PortId inputPort,
                     const data::DataPackage& data,
                     const segment::SegmentDataConsumers& consumers) final;

        void produce(segment::SegmentContext& segmentCtx,
                     const segment::SegmentDataConsumers& consumers) final;

        void setOperatorState(llvm::Value* operatorState);

    protected:
        virtual void build(segment::SegmentContext& segmentCtx,
                           shared::identifiers::PortId inputPort,
                           const data::DataPackage& data) = 0;
        virtual void probe(segment::SegmentContext& segmentCtx,
                           const segment::SegmentDataConsumers& consumers) = 0;

    private:
        llvm::Value* mOperatorState = nullptr;
    };
}  // namespace sierra::odyplan::v3::core::physical::operators::pipe

#endif  // SIERRA_ODYPLAN_V3_CORE_PHYSICAL_OPERATORS_PIPE_TYPED_BLOCKING_PIPE_OPERATOR_HPP