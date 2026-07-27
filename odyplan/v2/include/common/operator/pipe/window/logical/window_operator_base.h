#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_BASEOPERATOR_LOGICAL_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_BASEOPERATOR_LOGICAL_H

#include "common/window/window_type.h"
#include "core/logical/operator/pipe/pipe_operator.h"

namespace sierra::odyplan::v2::common::opr::pipe::logical {
    class WindowOperatorBase : public core::logical::opr::TypedPipeOperator {
    protected:
        template<typename T>
        static void checkBaseConfig(const window::WindowType windowType, const T windowSize, const T windowSlide) {
            if (windowSize == T{0}) {
                throw std::invalid_argument("window size cant be 0");
            }

            if (windowType == window::WindowType::TUMBLING && windowSlide != windowSize) {
                throw std::invalid_argument("window size and slide must be equal for TUMBLING");
            }

            if (windowType == window::WindowType::SLIDING && (windowSlide == T{0} || windowSlide > windowSize)) {
                throw std::invalid_argument("sliding requires 0 < slide <= size");
            }
        }
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_BASEOPERATOR_LOGICAL_H