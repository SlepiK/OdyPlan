#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_TIMEOPERATOR_PHYSICAL_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_TIMEOPERATOR_PHYSICAL_H

#include "window_operator_base.h"

namespace sierra::odyplan::v2::common::opr::pipe::physical {
    class TimeWindowOperator final : public WindowOperatorBase {
    public:
        TimeWindowOperator(core::physical::port::Port inputPort, core::physical::port::Port outputPort,
                           const std::uint64_t windowHandle, const std::size_t timestampIndex)
            : WindowOperatorBase(std::move(inputPort), std::move(outputPort), windowHandle),
              mTimestampIndex(timestampIndex) {
        }

    protected:
        llvm::Value *getTimestampValue(core::physical::segment::SegmentContext & /*segmentCtx*/,
                                       const core::physical::port::PortTuple &tuple, llvm::Type * /*i64Ty*/) override {
            return tuple.at(mTimestampIndex);
        }

    private:
        std::size_t mTimestampIndex;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_TIMEOPERATOR_PHYSICAL_H