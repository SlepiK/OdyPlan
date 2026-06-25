#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_OPERATORHANDLE_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_OPERATORHANDLE_H

#include "core/port/port.h"

namespace sierra::odyplan::v1::core::opr {
    template <typename Opt, port::PortsType TIns, port::PortType TOut>
    class OperatorHandle {
    public:
        using OperatorType = Opt;
        using OperatorInput = TIns;
        using OperatorOutput = TOut;

        explicit OperatorHandle(const std::size_t index) : mIndex(index) {}

        static constexpr std::size_t PORT_COUNT = TIns::SIZE;

        [[nodiscard]] std::size_t getIndex() const {
            return mIndex;
        }

    private:
        std::size_t mIndex;
    };

    template <typename Opt, typename TIn>
    struct OperatorHandleImpl;

    template <typename Opt>
    struct OperatorHandleImpl<Opt, port::NullPort> {
        using Type = OperatorHandle<Opt, port::NullPort, typename Opt::OperatorOutput>;
    };

    template <typename Opt, port::PortType TIns>
    struct OperatorHandleImpl<Opt, TIns> {
        using Type = OperatorHandle<Opt, port::Ports<TIns>, typename Opt::OperatorOutput>;
    };

    template <typename Opt, port::PortsType TIns>
    struct OperatorHandleImpl<Opt, TIns> {
        using Type = OperatorHandle<Opt, TIns, typename Opt::OperatorOutput>;
    };

    template <typename Opt>
    using HandleFromOperator = typename OperatorHandleImpl<Opt, typename Opt::OperatorInput>::Type;

}  // namespace sierra::odyplan::v1::core::opr

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_OPERATORHANDLE_H