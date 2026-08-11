#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_LOGICAL_TIME_WINDOW_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_LOGICAL_TIME_WINDOW_OPERATOR_HPP

#include <basic/operators/pipe/window/logical/base_window_operator.h>
#include <basic/window/time/time_window_config.h>
#include <utility>

namespace sierra::odyplan::v3::basic::operators::pipe::window::logical {
    class TimeWindowOperator final : public WindowOperatorBase {
    public:
        static constexpr core::shared::types::OperatorTypeId TYPE_ID =
            std::to_underlying(core::shared::types::BuiltinOperatorType::WINDOW_TIME);

        explicit TimeWindowOperator(const basic::window::time::TimeWindowConfig& windowConfig);

        [[nodiscard]] core::shared::types::OperatorTypeId getOperatorTypeId() const override;

        [[nodiscard]] const basic::window::time::TimeWindowConfig& getWindowConfig() const;

    protected:
        void validateInput() override;

    private:
        basic::window::time::TimeWindowConfig mWindowConfig;
    };
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::logical

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_LOGICAL_TIME_WINDOW_OPERATOR_HPP