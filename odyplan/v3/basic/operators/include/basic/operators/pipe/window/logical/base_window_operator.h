#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_LOGICAL_BASE_WINDOW_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_LOGICAL_BASE_WINDOW_OPERATOR_HPP

#include <stdexcept>
#include <basic/window/window_type.h>
#include <core/logical/operators/pipe/typed_pipe_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::logical {
    class WindowOperatorBase : public core::logical::operators::pipe::TypedPipeOperator {
    protected:
        template<typename T>
        static void checkBaseConfig(const basic::window::WindowType windowType, const T windowSize, const T windowSlide) {
            if (windowSize == T{0}) {
                throw std::invalid_argument("window size cant be 0");
            }

            if (windowType == basic::window::WindowType::TUMBLING && windowSlide != windowSize) {
                throw std::invalid_argument("window size and slide must be equal for TUMBLING");
            }

            if (windowType == basic::window::WindowType::SLIDING && (windowSlide == T{0} || windowSlide > windowSize)) {
                throw std::invalid_argument("sliding requires 0 < slide <= size");
            }
        }
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_LOGICAL_BASE_WINDOW_OPERATOR_HPP