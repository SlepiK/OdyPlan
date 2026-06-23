#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_OPERATORHANDLE_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_OPERATORHANDLE_H

#include "core/tuple/tuple.h"

namespace sierra::odyplan::v1::core::opr {
    template <tuple::Tuple TIn, tuple::Tuple TOut>
    class OperatorHandle {
    public:
        explicit OperatorHandle(std::size_t index) : mIndex(index) {}

        [[nodiscard]] std::size_t getIndex() const {
            return mIndex;
        }

    private:
        std::size_t mIndex;
    };
}  // namespace sierra::odyplan::v1::core::opr

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_OPERATORHANDLE_H