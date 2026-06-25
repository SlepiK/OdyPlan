#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_OPERATORHANDLE_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_OPERATORHANDLE_H

#include "core/port/port.h"

#include <typeindex>

namespace sierra::odyplan::v1::core::opr {
    class BaseOperatorHandle {
    public:
        virtual ~BaseOperatorHandle() = default;
        explicit BaseOperatorHandle(const std::size_t index) : mIndex(index) {}

        [[nodiscard]] std::size_t index() const {
            return mIndex;
        }

        [[nodiscard]] virtual std::type_index operatorType() const = 0;

    private:
        std::size_t mIndex;
    };

    template <typename T>
    concept HasOperatorHandleNeededFields =
        requires {
        typename T::Input;
        typename T::Output;
        typename T::OperatorType;
    };
}  // namespace sierra::odyplan::v1::core::opr

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_OPERATORHANDLE_H