#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_COUNTOPERATOR_PHYSICAL_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_COUNTOPERATOR_PHYSICAL_H

#include "window_operator_base.h"

namespace sierra::odyplan::v2::common::opr::pipe::physical {
    class CountWindowOperator final : public WindowOperatorBase {
    public:
        using WindowOperatorBase::WindowOperatorBase;

    protected:
        llvm::Value *getTimestampValue(core::physical::segment::SegmentContext & /*segmentCtx*/,
                                       const core::physical::port::PortTuple & /*tuple*/, llvm::Type *i64Ty) override {
            return llvm::ConstantInt::get(i64Ty, 0);
        }
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_COUNTOPERATOR_PHYSICAL_H