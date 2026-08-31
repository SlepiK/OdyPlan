#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_LOGICAL_BASE_WINDOW_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_LOGICAL_BASE_WINDOW_OPERATOR_HPP

#include <basic/window/window_type.h>
#include <core/logical/operators/pipe/typed_pipe_operator.h>
#include <stdexcept>

namespace sierra::odyplan::v3::basic::operators::pipe::window::logical {
    class WindowOperatorBase : public core::logical::operators::pipe::TypedPipeOperator {
    protected:
        template <typename T>
        static void checkBaseConfig(const basic::window::WindowType windowType,
                                    const T windowSize,
                                    const T windowSlide,
                                    const T windowAdvance) {
            if (windowSize <= T{}) {
                throw std::invalid_argument("window size must be positive");
            }

            if (windowType == basic::window::WindowType::SLIDING && windowSlide <= T{}) {
                throw std::invalid_argument("slide must be positive");
            }

            if (windowType == basic::window::WindowType::ADVANCE && windowAdvance <= T{}) {
                throw std::invalid_argument("advance must be positive");
                }
        }
    };
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::logical

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_LOGICAL_BASE_WINDOW_OPERATOR_HPP