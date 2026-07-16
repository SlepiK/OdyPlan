#ifndef SIERRA_ODYPLAN_V2_CORE_PHYSICAL_OPERATOR_PIPE_PIPEOPERATOR_H
#define SIERRA_ODYPLAN_V2_CORE_PHYSICAL_OPERATOR_PIPE_PIPEOPERATOR_H

#include <algorithm>

#include "core/physical/operator/operator.h"
#include "../operator_ability.h"
#include "core/physical/port/port.h"

namespace sierra::odyplan::v2::core::physical::opr {
    class IPipeOperator : public IOperator, public IConsumer {
    public:
        [[nodiscard]] bool hasInputPorts() const override {
            return true;
        }

        [[nodiscard]] bool hasOutputPorts() const override {
            return true;
        }

        [[nodiscard]] OperatorRole getRole() const final {
            return OperatorRole::PIPE;
        }
    };

    class TypedPipeOperator : public IPipeOperator {
    public:
        explicit TypedPipeOperator(port::Ports inputPorts, port::Ports outputPorts) {
            if (inputPorts.empty()) {
                throw std::invalid_argument("TypedPipeOperator: inputPorts must not be empty");
            }
            if (outputPorts.empty()) {
                throw std::invalid_argument("TypedPipeOperator: outputPorts must not be empty");
            }
            if (std::ranges::any_of(inputPorts, [](const port::Port& port) {return port.empty();})) {
                throw std::invalid_argument("TypedPipeOperator: no empty input port allowed");
            }
            if (std::ranges::any_of(outputPorts, [](const port::Port& port) {return port.empty();})) {
                throw std::invalid_argument("TypedPipeOperator: no empty output port allowed");
            }
            getInputPortsRef() = std::move(inputPorts);
            getOutputPortsRef() = std::move(outputPorts);
        }
    };

    class TypedBlockingPipeOperator : public TypedPipeOperator, public IProducer {
    public:
        explicit TypedBlockingPipeOperator(port::Ports inputPorts, port::Ports outputPorts)
            : TypedPipeOperator(std::move(inputPorts), std::move(outputPorts)) {
        }

        [[nodiscard]] bool isSegmentBreaker() const final {
            return true;
        }

        void consume(segment::SegmentContext& segmentCtx, const port::PortTuple &tuple, port::PortTupleConsumer  /*consume*/) final {
            build(segmentCtx, tuple);
        }

        void produce(segment::SegmentContext& segmentCtx, port::PortTupleConsumer consumer) final {
            probe(segmentCtx, std::move(consumer));
        }

        void setOperatorState(llvm::Value* operatorState) {
            mOperatorState = operatorState;
        }

    protected:
        virtual void build(segment::SegmentContext& segmentCtx, const port::PortTuple& tuple) = 0;
        virtual void probe(segment::SegmentContext& segmentCtx, port::PortTupleConsumer consumer) = 0;

        llvm::Value* mOperatorState = nullptr;
    };

    class TypedWindowPipeOperator : public TypedBlockingPipeOperator {
    public:
        enum class WindowType : uint8_t {
            TUMBLING,
            SLIDING
        };

        TypedWindowPipeOperator(port::Ports inputPorts, port::Ports outputPorts) : TypedBlockingPipeOperator(std::move(inputPorts), std::move(outputPorts)) {
        }

        [[nodiscard]] virtual WindowType getWindowType() const = 0;
    };
}  // namespace sierra::odyplan::v2::core::physical::opr

#endif  // SIERRA_ODYPLAN_V2_CORE_PHYSICAL_OPERATOR_PIPE_PIPEOPERATOR_H