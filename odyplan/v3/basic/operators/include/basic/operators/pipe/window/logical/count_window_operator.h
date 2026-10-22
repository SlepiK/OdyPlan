#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_LOGICAL_COUNT_WINDOW_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_LOGICAL_COUNT_WINDOW_OPERATOR_HPP

#include <basic/operators/pipe/window/details/count_window_options.h>
#include <utility>
#include <core/logical/operators/pipe/typed_pipe_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::logical {
    class CountWindowOperator final : public core::logical::operators::pipe::TypedPipeOperator {
    public:
        static constexpr core::shared::types::OperatorTypeId TYPE_ID =
            std::to_underlying(core::shared::types::BuiltinOperatorType::WINDOW_COUNT);

        explicit CountWindowOperator(const details::CountWindowOptions& windowConfig, bool assureOrder);

        [[nodiscard]] core::shared::types::OperatorTypeId getOperatorTypeId() const override;

        [[nodiscard]] const details::CountWindowOptions& getWindowConfig() const;

        [[nodiscard]] bool isOutOfOrder() const;
    protected:
        void validateInput() override;

    private:
        bool mAssureOrder;
        details::CountWindowOptions mWindowConfig;
    };
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::logical

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_LOGICAL_COUNT_WINDOW_OPERATOR_HPP