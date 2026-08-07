#include <core/physical/operators/pipe/typed_blocking_pipe_operator.h>

namespace sierra::odyplan::v3::core::physical::operators::pipe {
    TypedBlockingPipeOperator::TypedBlockingPipeOperator(port::Ports inputPorts, port::Ports outputPorts)
        : TypedPipeOperator(std::move(inputPorts), std::move(outputPorts)) {
    }

    bool TypedBlockingPipeOperator::isSegmentBreaker() const {
        return true;
    }

    void TypedBlockingPipeOperator::consume(segment::SegmentContext& segmentCtx, const shared::identifiers::PortId inputPort, const data::DataPackage& data, const segment::SegmentDataConsumers&  /*consumers*/) {
        build(segmentCtx, inputPort, data);
    }

    void TypedBlockingPipeOperator::produce(segment::SegmentContext& segmentCtx, const segment::SegmentDataConsumers& consumers) {
        probe(segmentCtx, consumers);
    }

    void TypedBlockingPipeOperator::setOperatorState(llvm::Value* operatorState) {
        mOperatorState = operatorState;
    }

}
