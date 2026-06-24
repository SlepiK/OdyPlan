#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_OPERATORHANDLE_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_OPERATORHANDLE_H

#include "core/tuple/tuple.h"

namespace sierra::odyplan::v1::core::opr {
    template <typename Opt, tuple::Tuple TOut, tuple::Tuple... TIn>
    class OperatorHandle {
    public:
        explicit OperatorHandle(const std::size_t index) : mIndex(index) {}

        static constexpr std::size_t PORT_COUNT = sizeof...(TIn);

        [[nodiscard]] std::size_t getIndex() const {
            return mIndex;
        }

    private:
        std::size_t mIndex;
    };
}  // namespace sierra::odyplan::v1::core::opr

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_OPERATORHANDLE_H